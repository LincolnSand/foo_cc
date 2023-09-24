typedef struct foo foo1_t;
typedef foo1_t foo2_t;

struct foo {
    int f;
    float t;
};

int main() {
    foo2_t f1;
    foo1_t f2;
    struct foo f3;
    //struct foo_t f;

    int v = 7;
    if(v) {
        double ll = 0.7f;
    }
}
