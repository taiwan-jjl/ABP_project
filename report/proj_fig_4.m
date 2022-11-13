%%
%
x = [1,2,3,4,5,6];
perf_32_f = [25.4593, 48.7752, 76.1958, 81.8784, 98.037, 117.241];
perf_64_f = [20.3676, 33.5496, 40.3462, 42.4118, 43.2959, 43.7574];
perf_128_f = [20.1538, 31.8469, 36.0334, 36.5959, 36.6378, 36.8494];
perf_256_f = [20.21, 31.1432, 35.1259, 35.6628, 36.0668, 35.6336];
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
title("Task-1.1, CRS format, CPU strong scaling test, double");
legend("N=32", "N=64", "N=128", "N=256", "CPU-memory-1ch=23.5GB/s", "CPU-memory-2ch=47GB/s");


%%
%
plot(x, perf_32_f_r, "-x", x, perf_64_f_r, "-x", x, perf_128_f_r, "-x", x, perf_256_f_r, "-x");
xlim([0 7])
xlabel("# of CPU cores");
ylabel("speed-up ratio");
title("Task-1.1, CRS format, CPU strong scaling test, double");
legend("N=32", "N=64", "N=128", "N=256");
