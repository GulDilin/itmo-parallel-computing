kernel void memset(   global uint *dst ) {
    dst[get_global_id(0)] = get_global_id(0) * 2;
}
