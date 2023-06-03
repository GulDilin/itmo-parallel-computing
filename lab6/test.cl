kernel void memset(
    global double *dst
) {
    dst[get_global_id(0)] = get_global_id(0) * 2;
}


kernel void merge_sorted(
    global double *src,
    global double *dst,
    int offset_1,
    int offset_2,
    int offset_dst,
    int n_src_1,
    int n_src_2
) {
    int i1 = offset_1;
    int i2 = offset_2;
    int i = offset_dst;
    while (i < n_src_1 + n_src_2) {
        dst[i++] = src[i1] > src[i2] && i2 < n_src_2 + offset_2 ? src[i2++] : src[i1++];
    }
}
