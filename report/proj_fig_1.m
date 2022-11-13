%%
%
x = [1,2,3,4,5,6];
perf_32_f = [69.2209, 140.952, 203.812, 243.883, 304.689, 290.189];
perf_64_f = [60.1206, 112.203, 147.657, 166.014, 179.086, 181.521];
perf_128_f = [58.6648, 101.886, 125.108, 135.836, 136.03, 139.278];
perf_256_f = [57.955, 98.7807, 119.286, 128.999, 129.639, 130.426];
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
title("Task-1.1, CRS format, CPU strong scaling test, float");
legend("N=32", "N=64", "N=128", "N=256");


%%
%
plot(x, perf_32_f_r, "-x", x, perf_64_f_r, "-x", x, perf_128_f_r, "-x", x, perf_256_f_r, "-x");
xlim([0 7])
xlabel("# of CPU cores");
ylabel("speed-up ratio");
title("Task-1.1, CRS format, CPU strong scaling test, float");
legend("N=32", "N=64", "N=128", "N=256");
