kernel void memset(
    global double *dst
) {
    dst[get_global_id(0)] = get_global_id(0) * 2;
}

kernel void pow_log10(
    global double *src,
    global double *dst
) {
    int i = get_global_id(0);
    dst[i] = pow(log10(src1[i]), M_E);
}
