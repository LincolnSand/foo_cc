//typedef struct foo foo_t;

typedef struct foo {
    int f;
    float t;
} foo_t;

int main() {
    foo_t f1;
    struct foo f2;
    //struct foo_t f;

    int v = 7;
    if(v) {
        double ll = 0.7f;
    }
}
