int main() {
    float normal_float = 123.234;

    float exp_float = 123.234e12;

    float sign_exp_float = 123.234e+11;
    sign_exp_float = 123.234e-11;


    double suffixed_float = 1.123f;
    double suffixed_exp_float = 12312e12L;
    double suffixed_sign_exp_float = 123.1232e-123f;
    suffixed_sign_exp_float = 123.6787e+123f;

    float hex_float = 0x123p12;
    float suffixed_hex_float = 0x123P12f;
    float sign_exp_hex_float = 0x123p-12;
    sign_exp_hex_float = 0x123p+12;
    float sign_exp_suffixed_hex_float = 0x123p12f;
    sign_exp_suffixed_hex_float = 0x123p+12f;
    sign_exp_suffixed_hex_float = 0x123p-12f;
    float float_end_dot = 1231.;
    float float_end_dot_suffix = 132123.f;
    float_end_dot_suffix = 132123.l;
    float_end_dot_suffix = 132123.F;
    float_end_dot_suffix = 132123.l;

    float float_dot_start = .123123;
    float suffix_float_dot_start = .123123f;
    suffix_float_dot_start = .123123F;
    suffix_float_dot_start = .123123l;
    suffix_float_dot_start = .123123L;

    float exp_float_dot_start = .1123123e123;
    double sign_exp_float_dot_start = .1123123e+123;
    sign_exp_float_dot_start = .1123123e-123;
    double sign_exp_float_dot_start_suffix = .1123123e+123f;
    sign_exp_float_dot_start_suffix = .1123123e+123l;
    sign_exp_float_dot_start_suffix = .1123123e+123L;
    sign_exp_float_dot_start_suffix = .1123123e+123F;

    int normal_int = 123556;
    int suffixed_int = 2309ULL;
    suffixed_int = 12123UL;
    suffixed_int = 12123L;
    suffixed_int = 12123LU;
    suffixed_int = 12123LLU;
    suffixed_int = 12123llU;
    suffixed_int = 12123llu;
    suffixed_int = 12123lu;

    int hex_int = 0x123;
    hex_int = 0X123;

    int suffixed_hex_int = 0x123123ULL;
    suffixed_hex_int = 0x123123ull;
    suffixed_hex_int = 0x123123uL;
    suffixed_hex_int = 0x123123LLU;
    suffixed_hex_int = 0x123123LLu;
    suffixed_hex_int = 0x123123lu;
    suffixed_hex_int = 0x123123llu;

    int octal_int = 0123123;
    int suffixed_octal_int = 0123123U;
    suffixed_octal_int = 0123123Ul;
    suffixed_octal_int = 0123123ULL;
    suffixed_octal_int = 0123123uLL;
    suffixed_octal_int = 0123123uL;
    suffixed_octal_int = 0123123ul;
    suffixed_octal_int = 0123123ull;
}