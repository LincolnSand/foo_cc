typedef struct foo foo1_t;
typedef foo1_t foo2_t;

typedef struct {
    int f;
    float t;
} foo_t;

int main() {
    foo_t f1;
    foo_t f2;
    f1.f = 9;
    f2.f = f1.f;

    f2;

    int v = 7;
    if(v) {
        double ll = 0.7f;
    }

    return f2.f;
}
