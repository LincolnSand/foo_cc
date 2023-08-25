int main() {
    double e = 2.71828;
    char c = 'c';
    int i = 901;
    double j = 1000 * i;
    i = 9; // Since assignment is technically a binary operator, mismatched types are unsupported, unlike for declarations (where the rhs is implicitly converted to the lhs's type).
    double k = i + 2;
}
