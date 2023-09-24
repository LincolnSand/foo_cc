struct f1 {
    int i;
};

struct f2 {
    struct f1 f;
};

struct f3 {
    struct f2 f;
};

int main() {
    struct f3 f;
    f.f.f.i = 9;
    return f.f.f.i;
}

