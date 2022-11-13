%%
%
x = [1,2,3,4,5,6];
perf_32_f = [62.0241, 126.568, 191.004, 177.883, 297.263, 277.803];
perf_64_f = [50.36, 84.6826, 100.525, 104.354, 107.541, 108.948];
perf_128_f = [47.9142, 71.9351, 83.5773, 84.2394, 84.1857, 84.7066];
perf_256_f = [47.0583, 71.6761, 80.2801, 81.2249, 81.9354, 81.0856];
perf_32_f_r = perf_32_f ./ perf_32_f(1);
perf_64_f_r = perf_64_f ./ perf_64_f(1);
perf_128_f_r = perf_128_f ./ perf_128_f(1);
perf_256_f_r = perf_256_f ./ perf_256_f(1);


%%
%
plot(x, perf_32_f, "-x", x, perf_64_f, "-x", x, perf_128_f, "-x", x, perf_256_f, "-x");
xlim([0 7])
xlabel("# of CPU cores");
ylabel("MUPD/s/it");
title("Task-1.1, CRS format, CPU strong scaling test, double");
legend("N=32", "N=64", "N=128", "N=256");


%%
%
plot(x, perf_32_f_r, "-x", x, perf_64_f_r, "-x", x, perf_128_f_r, "-x", x, perf_256_f_r, "-x");
xlim([0 7])
xlabel("# of CPU cores");
ylabel("speed-up ratio");
title("Task-1.1, CRS format, CPU strong scaling test, double");
legend("N=32", "N=64", "N=128", "N=256");
