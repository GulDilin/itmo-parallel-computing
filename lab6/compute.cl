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

kernel void pow_log10(
    global double *src,
    global double *dst
) {
    int i = get_global_id(0);
    dst[i] = pow(log10(src[i]), M_E);
}

kernel void max_2_src(
    global double *src1,
    global double *src2,
    global double *dst
) {
    int i = get_global_id(0);
    dst[i] = max(src1[i], src2[i]);
}

kernel void sort(
    global double *src_offset,
    global double *src_size,
    global double *src
) {
    int j = get_global_id(0);
    int offset = src_offset[j];
    int i = 0;
    while (i < src_size[i] - 1) {
        if (src[offset + i + 1] < src[offset + i]) {
            double t = src[offset + i + 1];
            src[offset + i + 1] = src[offset + i];
            src[offset + i] = t;
            i = 0;
        } else {
            i++;
        }
    }
}

kernel void merge_sorted(
    global double *src,
    global double *dst,
    global int *offset_and_sizes
) {
    int i1 = offset_and_sizes[0];
    int i2 = offset_and_sizes[1];
    int i = offset_and_sizes[2];
    int n_src_1 = offset_and_sizes[3];
    int n_src_2 = offset_and_sizes[4];
    int offset_2 = offset_and_sizes[1];
    while (i < n_src_1 + n_src_2) {
        dst[i++] = src[i1] > src[i2] && i2 < n_src_2 + offset_2 ? src[i2++] : src[i1++];
    }
}
