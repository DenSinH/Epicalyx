typedef struct s_struct{
    int a, b;
    void (*c)(int);
} s_struct;

s_struct my_function() {
    return (s_struct){
        .a = 1,
        .b = 2,
        .c = (void (*)(int))0
    };
}