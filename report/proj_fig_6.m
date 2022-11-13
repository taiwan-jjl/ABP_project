%%
%
x = [32, 64, 128];
perf_32_f = [103.276, 97.7538, 95.978];
perf_64_f = [107.748, 103.309, 98.8546];
perf_128_f = [87.8309, 48.9657, 37.0415];
perf_256_f = [117.241, 43.7574, 36.8494];

% perf_32_f_r = perf_32_f ./ perf_32_f(1);
% perf_64_f_r = perf_64_f ./ perf_64_f(1);
% perf_128_f_r = perf_128_f ./ perf_128_f(1);
% perf_256_f_r = perf_256_f ./ perf_256_f(1);


%%
%
plot(x, perf_32_f, "-x", x, perf_64_f, "-x", x, perf_128_f, "-x", x, perf_256_f, "-x");
yline(46.928);
yline(46.928/2);
yline(264);
%xlim([0 7])
xlabel("N");
ylabel("GB/s");
title("Task-2, CRS format, GPU");
legend("GPU-float", "GPU-double", "CPU-6C-float", "CPU-6C-double", "CPU-memory-1ch=23.5GB/s", "CPU-memory-2ch=47GB/s", "GDDR6-192bit=264GB/s");


%%
%
plot(x, perf_32_f_r, "-x", x, perf_64_f_r, "-x", x, perf_128_f_r, "-x", x, perf_256_f_r, "-x");
xlim([0 7])
xlabel("# of CPU cores");
ylabel("speed-up ratio");
title("Task-1.1, CRS format, CPU strong scaling test, double");
legend("N=32", "N=64", "N=128", "N=256");
