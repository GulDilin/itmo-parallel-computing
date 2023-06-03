kernel void memset(
    global double *dst
) {
    dst[get_global_id(0)] = get_global_id(0) * 2;
}

kernel void ctanh_sqrt(
    global double *src,
    global double *dst
) {
    int i = get_global_id(0);
    dst[i] = 1 / tanh(sqrt(src[i]));
}

kernel void sum_prev(
    global double *src1,
    global double *src2,
    global double *dst
) {
    int i = get_global_id(0);
    dst[i] = i > 0 ? src1[i] + src2[i - 1] : src1[i];
}
