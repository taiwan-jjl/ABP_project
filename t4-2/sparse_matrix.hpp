
#ifndef sparse_matrix_hpp
#define sparse_matrix_hpp

#include <utility>

#ifdef HAVE_MPI
#  include <mpi.h>
#endif

#include <omp.h>

#include <vector>

#include "vector.hpp"


#ifndef DISABLE_CUDA
template <typename Number>
__global__ void compute_spmv(const std::size_t N,
                             const std::size_t *row_starts,
                             const unsigned int *column_indices,
                             const Number *values,
                             const Number *src,
                             Number *dst)
{
  // TODO implement for GPU
  // const int idx = row_starts[blockIdx.x] + threadIdx.x;
  // const int idx_end = row_starts[blockIdx.x+1];
  // Number sum = 0;
  // if(idx < idx_end){
  //   sum += values[idx] * x[column_indices[idx]];
  // }
  // atomicAdd(&y[blockIdx.x], sum);

  // const int row = threadIdx.x + blockDim.x*blockIdx.x;
  // if(row<N){
  //   Number sum = 0;
  //   for (std::size_t idx = row_starts[row]; idx < row_starts[row + 1]; ++idx)
  //   {
  //     sum += values[idx] * src[column_indices[idx]];
  //   }
  //   dst[row] = sum;
  // }




  // For SELL-C method
  // printf("sell-c begin here\n");
  const size_t row = threadIdx.x + blockDim.x*blockIdx.x;
  //printf("n_rows = %lu, row = %lu\n", N, row);
  if(row<N){
    Number sum = 0;
    const size_t warp = 32;
    const size_t n_blocks = (N + warp -1) / warp;
    size_t block_width = row_starts[row+1] - row_starts[row];



    size_t idx_blocks = row / warp;
    size_t idx_offset = 0;
    for(size_t i=0; i<idx_blocks;i++){idx_offset = idx_offset + warp*(row_starts[i*warp+1]-row_starts[i*warp]);}
    size_t idx_row = row % warp;
    size_t warp_last = N % warp;

    if(row >= (n_blocks-1)*warp){
      if(warp_last == 0){
        for (size_t idx = idx_offset + idx_row; idx < idx_offset + idx_row + (block_width-1)*warp; idx=idx+warp)
      {
        sum += values[idx] * src[column_indices[idx]];
      }
      }else{
        for (size_t idx = idx_offset + idx_row; idx < idx_offset + idx_row + (block_width-1)*warp_last; idx=idx+warp_last)
      {
        sum += values[idx] * src[column_indices[idx]];
      }
      }

    }else{
      for (size_t idx = idx_offset + idx_row; idx < idx_offset + idx_row + (block_width-1)*warp; idx=idx+warp)
      {
        sum += values[idx] * src[column_indices[idx]];
      }
    }

    


    dst[row] = sum;
  }

  // printf("sell-c end here\n");


}
#endif



// Sparse matrix in compressed row storage (crs) format

template <typename Number>
class SparseMatrix
{
public:
  static const int block_size = Vector<Number>::block_size;

  SparseMatrix(const std::vector<unsigned int> &row_lengths,
               const MemorySpace                memory_space,
               const MPI_Comm                   communicator)
    : communicator(communicator),
      memory_space(memory_space)
  {
    n_rows     = row_lengths.size();
    row_starts = new std::size_t[n_rows + 1];

#pragma omp parallel for
    for (unsigned int row = 0; row < n_rows + 1; ++row)
      row_starts[row] = 0;

    for (unsigned int row = 0; row < n_rows; ++row)
      row_starts[row + 1] = row_starts[row] + row_lengths[row];

    const std::size_t n_entries = row_starts[n_rows];

    if (memory_space == MemorySpace::CUDA)
      {
        std::size_t *host_row_starts = row_starts;
        row_starts = 0;
        AssertCuda(cudaMalloc(&row_starts, (n_rows + 1) * sizeof(std::size_t)));
        AssertCuda(cudaMemcpy(row_starts,
                              host_row_starts,
                              (n_rows + 1) * sizeof(std::size_t),
                              cudaMemcpyHostToDevice));
        delete[] host_row_starts;

        AssertCuda(cudaMalloc(&column_indices,
                              n_entries * sizeof(unsigned int)));
        AssertCuda(cudaMalloc(&values, n_entries * sizeof(Number)));

#ifndef DISABLE_CUDA
        const unsigned int n_blocks =
          (n_entries + block_size - 1) / block_size;
        set_entries<<<n_blocks, block_size>>>(n_entries, 0U, column_indices);
        set_entries<<<n_blocks, block_size>>>(n_entries, Number(0), values);
        AssertCuda(cudaPeekAtLastError());
#endif
      }
    else
      {
        column_indices = new unsigned int[n_entries];
        values         = new Number[n_entries];

#pragma omp parallel for
        for (std::size_t i = 0; i < n_entries; ++i)
          column_indices[i] = 0;

#pragma omp parallel for
        for (std::size_t i = 0; i < n_entries; ++i)
          values[i] = 0;
      }

    n_global_nonzero_entries = mpi_sum(n_entries, communicator);
  }

  ~SparseMatrix()
  {
    if (memory_space == MemorySpace::CUDA)
      {
#ifndef DISABLE_CUDA
        cudaFree(row_starts);
        cudaFree(column_indices);
        cudaFree(values);
#endif
      }
    else
      {
        delete[] row_starts;
        delete[] column_indices;
        delete[] values;
      }
  }

  SparseMatrix(const SparseMatrix &other)
    : communicator(other.communicator),
      memory_space(other.memory_space),
      n_rows(other.n_rows),
      n_global_nonzero_entries(other.n_global_nonzero_entries)
  {
    if (memory_space == MemorySpace::CUDA)
      {
        AssertCuda(cudaMalloc(&row_starts, (n_rows + 1) * sizeof(std::size_t)));
        AssertCuda(cudaMemcpy(row_starts,
                              other.row_starts,
                              (n_rows + 1) * sizeof(std::size_t),
                              cudaMemcpyDeviceToDevice));

        std::size_t n_entries = 0;
        AssertCuda(cudaMemcpy(&n_entries,
                              other.row_starts + n_rows,
                              sizeof(std::size_t),
                              cudaMemcpyDeviceToHost));
        AssertCuda(cudaMalloc(&column_indices,
                              n_entries * sizeof(unsigned int)));
        AssertCuda(cudaMemcpy(column_indices,
                              other.column_indices,
                              n_entries * sizeof(unsigned int),
                              cudaMemcpyDeviceToDevice));

        AssertCuda(cudaMalloc(&values, n_entries * sizeof(Number)));
        AssertCuda(cudaMemcpy(values,
                              other.values,
                              n_entries * sizeof(Number),
                              cudaMemcpyDeviceToDevice));
      }
    else
      {

      }
  }

  // do not allow copying matrix
  SparseMatrix operator=(const SparseMatrix &other) = delete;

  unsigned int m() const
  {
    return n_rows;
  }

  std::size_t n_nonzero_entries() const
  {
    return n_global_nonzero_entries;
  }

  void add_row(unsigned int               row,
               std::vector<unsigned int> &columns_of_row,
               std::vector<Number> &      values_in_row)
  {
    if (columns_of_row.size() != values_in_row.size())
      {
        std::cout << "column_indices and values must have the same size!"
                  << std::endl;
        std::abort();
      }
    for (unsigned int i = 0; i < columns_of_row.size(); ++i)
      {
        column_indices[row_starts[row] + i] = columns_of_row[i];
        values[row_starts[row] + i]         = values_in_row[i];
      }
  }

  void allocate_ghost_data_memory(const std::size_t n_ghost_entries)
  {
    ghost_entries.clear();
    ghost_entries.reserve(n_ghost_entries);
#pragma omp parallel for
    for (unsigned int i = 0; i < n_ghost_entries; ++i)
      {
        ghost_entries[i].index_within_result         = 0;
        ghost_entries[i].index_within_offproc_vector = 0;
        ghost_entries[i].value                       = 0.;
      }
  }

  void add_ghost_entry(const unsigned int local_row,
                       const unsigned int offproc_column,
                       const Number       value)
  {
    GhostEntryCoordinateFormat entry;
    entry.value                       = value;
    entry.index_within_result         = local_row;
    entry.index_within_offproc_vector = offproc_column;
    ghost_entries.push_back(entry);
  }

  // In real codes, the data structure we pass in manually here could be
  // deduced from the global indices that are accessed. In the most general
  // case, it takes some two-phase index lookup via a dictionary to find the
  // owner of particular columns (sometimes called consensus algorithm).
  void set_send_and_receive_information(
    std::vector<std::pair<unsigned int, std::vector<unsigned int>>>
                                                       send_indices,
    std::vector<std::pair<unsigned int, unsigned int>> receive_indices)
  {
    this->send_indices    = send_indices;
    std::size_t send_size = 0;
    for (auto i : send_indices)
      send_size += i.second.size();
    send_data.resize(send_size);
    this->receive_indices    = receive_indices;
    std::size_t receive_size = 0;
    for (auto i : receive_indices)
      receive_size += i.second;
    receive_data.resize(receive_size);

    const unsigned int my_mpi_rank = get_my_mpi_rank(communicator);

    if (receive_size > ghost_entries.size())
      {
        std::cout << "Error, you requested exchange of more entries than what "
                  << "there are ghost entries allocated in the matrix, which "
                  << "does not make sense. Check matrix setup." << std::endl;
        std::abort();
      }
  }


  //For matrix transformation.
  //do it in cpu
  void transmethod()
  {

    // printf("******debug\n");
    // printf("n_rows = %lu\n", n_rows);
    // printf("\n");
    // values[0] = 999.9;


    const size_t warp = 32;
    const size_t n_blocks = (n_rows + warp -1) / warp;
    // size_t new_n_rows = warp*n_blocks;
    // printf("n_blocks = %lu \n", n_blocks);
    // printf("new_n_rows = %lu \n", new_n_rows)
    // printf("\n");

    size_t new_array_len = 0;
    size_t block_width[n_blocks] = {0};

    for(size_t i=0; i<n_blocks; i++){
      size_t max_len = 0;
      size_t len = 0;
      for(size_t j=i*warp; j<(i+1)*warp; j++){
        if(j<n_rows){
          // printf("row_starts = %lu \n", row_starts[j]);
          // printf("row_len = %lu \n", row_starts[j+1] - row_starts[j]);
          len = row_starts[j+1] - row_starts[j];
          if(max_len < len){
            max_len = len;
          }
        }
      }
      // printf("max_len = %lu \n\n", max_len);
      block_width[i] = max_len;
      if(i == n_blocks-1){
        if(0 == n_rows % warp){
          new_array_len = new_array_len + warp*max_len;
        }else{
          new_array_len = new_array_len + (n_rows % warp) * max_len;
        }        
      }else{
        new_array_len = new_array_len + warp*max_len;
      }
      
    }
    // printf("new_array_len = %lu \n", new_array_len);
    // for(size_t i=0; i<n_blocks; i++){printf("block_width[%lu] = %lu\n", i, block_width[i]);}

    Number *new_values = new Number[new_array_len]();
    unsigned int *new_col_indices = new unsigned int[new_array_len]();

    for (size_t row = 0; row < n_rows; ++row)
      {
        size_t idx_blocks = row / warp;
        size_t idx_offset = 0;
        for(size_t i=0; i<idx_blocks;i++){idx_offset = idx_offset + warp*block_width[i];}
        size_t idx_row = row % warp;
        size_t idx_idx = 0;
        size_t warp_last = n_rows % warp;
        
        for (size_t idx = row_starts[row]; idx < row_starts[row + 1]; ++idx)
        {
          if(row >= (n_blocks-1)*warp){
            new_values[idx_offset + idx_row + idx_idx*warp_last] = values[idx];
            new_col_indices[idx_offset + idx_row + idx_idx*warp_last] = column_indices[idx];
            idx_idx = idx_idx + 1;
          }else{
            new_values[idx_offset + idx_row + idx_idx*warp] = values[idx];
            new_col_indices[idx_offset + idx_row + idx_idx*warp] = column_indices[idx];
            idx_idx = idx_idx + 1;
          }
        }
      }


    // for(int i=0; i<n_global_nonzero_entries; i++){
    //   printf("value[%i] = %f\n", i, values[i]);
    // }
    // for(int i=0; i<new_array_len; i++){
    //   printf("new_values[%i] = %f\n", i, new_values[i]);
    // }
    // for (size_t row = 0; row < n_rows; ++row)
    //   {
    //     for (size_t idx = row_starts[row]; idx < row_starts[row + 1]; ++idx)
    //     {
    //       printf("value[%lu] = %f\n", idx, values[idx]);
    //     }
    //   }
      

    values = new_values;
    column_indices = new_col_indices;
    row_starts[n_rows] = new_array_len;
    n_global_nonzero_entries = new_array_len;
    
    row_starts[0] = 0;
    for(size_t i=1; i<n_rows; i++){
      row_starts[i] = row_starts[i-1] + block_width[(i-1)/warp];
      // printf("row_starts[%lu] = %lu\n", i, row_starts[i]);
    }

    // printf("trans end\n");
    // trans<<<1, 1>>>(n_rows,
    //                 row_starts,
    //                 column_indices,
    //                 values);
  }




  void apply(const Vector<Number> &src, Vector<Number> &dst) const
  {
    if (m() != src.size_on_this_rank() || m() != dst.size_on_this_rank())
      {
        std::cout << "vector sizes of src " << src.size_on_this_rank()
                  << " and dst " << dst.size_on_this_rank()
                  << " do not match matrix size " << m() << std::endl;
        std::abort();
      }

#ifdef HAVE_MPI
    // start exchanging the off-processor data
    std::vector<MPI_Request> mpi_requests(send_indices.size() +
                                          receive_indices.size());
    for (unsigned int i = 0, count = 0; i < receive_indices.size();
         count += receive_indices[i].second, ++i)
      MPI_Irecv(receive_data.data() + count,
                receive_indices[i].second * sizeof(Number),
                MPI_BYTE,
                receive_indices[i].first,
                /* mpi_tag */ 29,
                communicator,
                &mpi_requests[i]);
    for (unsigned int i = 0, count = 0; i < send_indices.size(); ++i)
      {
#  pragma omp parallel for
        for (unsigned int j = 0; j < send_indices[i].second.size(); ++j)
          send_data[count + j] = src(send_indices[i].second[j]);

        MPI_Isend(send_data.data() + count,
                  send_indices[i].second.size() * sizeof(Number),
                  MPI_BYTE,
                  send_indices[i].first,
                  /* mpi_tag */ 29,
                  communicator,
                  &mpi_requests[i + receive_indices.size()]);
        count += send_indices[i].second.size();
      }
#endif

    // main loop for the sparse matrix-vector product
    if (memory_space == MemorySpace::CUDA)
      {
#ifndef DISABLE_CUDA
        // TODO implement for GPU (with CRS and ELLPACK/SELL-C-sigma)
        const unsigned int n_blocks = (n_rows + block_size - 1) / block_size;
        compute_spmv<<<n_blocks, block_size>>>(n_rows,
                                               row_starts,
                                               column_indices,
                                               values,
                                               src.begin(),
                                               dst.begin());




        AssertCuda(cudaPeekAtLastError());
#endif
      }
    else
      {
#pragma omp parallel for
        for (unsigned int row = 0; row < n_rows; ++row)
          {
            Number sum = 0;
            for (std::size_t idx = row_starts[row]; idx < row_starts[row + 1];
                 ++idx)
              sum += values[idx] * src(column_indices[idx]);
            dst(row) = sum;
          }
      }

#ifdef HAVE_MPI
    MPI_Waitall(mpi_requests.size(), mpi_requests.data(), MPI_STATUSES_IGNORE);

    // work on the off-processor data. do not do it in parallel because we do
    // not know whether two parts would work on the same entry of the result
    // vector
    for (auto &entry : ghost_entries)
      dst(entry.index_within_result) +=
        entry.value * receive_data[entry.index_within_offproc_vector];
#endif
  }

  SparseMatrix copy_to_device()
  {
    if (memory_space == MemorySpace::CUDA)
      {
        std::cout << "Copy between device matrices not implemented"
                  << std::endl;
        exit(EXIT_FAILURE);
        // return dummy
        return SparseMatrix(std::vector<unsigned int>(),
                            MemorySpace::CUDA,
                            communicator);
      }
    else
      {
        std::vector<unsigned int> row_lengths(n_rows);
        for (unsigned int i = 0; i < n_rows; ++i)
          row_lengths[i] = row_starts[i + 1] - row_starts[i];

        SparseMatrix other(row_lengths,
                           MemorySpace::CUDA,
                           communicator);
        AssertCuda(cudaMemcpy(other.column_indices,
                              column_indices,
                              row_starts[n_rows] * sizeof(unsigned int),
                              cudaMemcpyHostToDevice));
        AssertCuda(cudaMemcpy(other.values,
                              values,
                              row_starts[n_rows] * sizeof(Number),
                              cudaMemcpyHostToDevice));
        return other;
      }
  }

  std::size_t memory_consumption() const
  {
    return n_global_nonzero_entries * (sizeof(Number) + sizeof(unsigned int)) +
           (n_rows + 1) * sizeof(decltype(*row_starts)) +
           sizeof(GhostEntryCoordinateFormat) * ghost_entries.capacity();
  }

private:
  MPI_Comm      communicator;
  std::size_t   n_rows;
  std::size_t * row_starts;
  unsigned int *column_indices;
  Number *      values;
  std::size_t   n_global_nonzero_entries;
  MemorySpace   memory_space;

  struct GhostEntryCoordinateFormat
  {
    unsigned int index_within_result;
    unsigned int index_within_offproc_vector;
    Number       value;
  };
  std::vector<GhostEntryCoordinateFormat> ghost_entries;

  std::vector<std::pair<unsigned int, std::vector<unsigned int>>> send_indices;
  mutable std::vector<Number>                                     send_data;
  std::vector<std::pair<unsigned int, unsigned int>> receive_indices;
  mutable std::vector<Number>                        receive_data;
};


#endif