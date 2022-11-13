%%
%
x = [1,2,3,4,5,6];
perf_32_f = [18.5418, 37.3663, 53.796, 64.18, 74.4462, 87.8309];
perf_64_f = [16.222, 30.1598, 39.6326, 44.4039, 48.6054, 48.9657];
perf_128_f = [15.9129, 28.1996, 34.3728, 36.9191, 36.7169, 37.0415];
perf_256_f = [15.8744, 27.6949, 33.75, 36.238, 36.2878, 36.4507];
perf_32_f_r = perf_32_f ./ perf_32_f(1);
perf_64_f_r = perf_64_f ./ perf_64_f(1);
perf_128_f_r = perf_128_f ./ perf_128_f(1);
perf_256_f_r = perf_256_f ./ perf_256_f(1);


%%
%
plot(x, perf_32_f, "-x", x, perf_64_f, "-x", x, perf_128_f, "-x", x, perf_256_f, "-x");
yline(46.928);
yline(46.928/2);
xlim([0 7])
xlabel("# of CPU cores");
ylabel("GB/s");
title("Task-1.1, CRS format, CPU strong scaling test, float");
legend("N=32", "N=64", "N=128", "N=256", "CPU-memory-1ch=23.5GB/s", "CPU-memory-2ch=47GB/s");


%%
%
plot(x, perf_32_f_r, "-x", x, perf_64_f_r, "-x", x, perf_128_f_r, "-x", x, perf_256_f_r, "-x");
xlim([0 7])
xlabel("# of CPU cores");
ylabel("speed-up ratio");
title("Task-1.1, CRS format, CPU strong scaling test, float");
legend("N=32", "N=64", "N=128", "N=256");
