extern "C" {

__attribute__((visibility("default"))) int probe_version() {
  return 1;
}

__attribute__((visibility("default"))) int format_probe(
    const unsigned char* input,
    int input_len,
    unsigned char* output,
    int output_cap) {
  if (input_len < 0 || output_cap < 0) {
    return -1;
  }

  int written = 0;
  for (; written < input_len && written < output_cap; ++written) {
    output[written] = input[written];
  }

  if (written < output_cap) {
    output[written] = '\n';
    ++written;
  }

  return written;
}

}
