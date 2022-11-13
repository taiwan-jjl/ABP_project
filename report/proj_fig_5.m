%%
%
x = [32, 64, 128];
perf_32_f = [292.21, 378.007, 378.767];
perf_64_f = [227.097, 258.025, 250.541];
perf_128_f = [290.189, 181.521, 139.278];
perf_256_f = [277.803, 108.948, 84.7066];

% perf_32_f_r = perf_32_f ./ perf_32_f(1);
% perf_64_f_r = perf_64_f ./ perf_64_f(1);
% perf_128_f_r = perf_128_f ./ perf_128_f(1);
% perf_256_f_r = perf_256_f ./ perf_256_f(1);


%%
%
plot(x, perf_32_f, "-x", x, perf_64_f, "-x", x, perf_128_f, "-x", x, perf_256_f, "-x");
xlabel("N");
ylabel("MUPD/s/it");
title("Task-2, CRS format, GPU");
legend("GPU-float", "GPU-double", "CPU-6C-float", "CPU-6C-double");


%%
%
plot(x, perf_32_f_r, "-x", x, perf_64_f_r, "-x", x, perf_128_f_r, "-x", x, perf_256_f_r, "-x");
xlim([0 7])
xlabel("# of CPU cores");
ylabel("speed-up ratio");
title("Task-1.1, CRS format, CPU strong scaling test, double");
legend("N=32", "N=64", "N=128", "N=256");
