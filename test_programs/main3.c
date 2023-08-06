int main() {
    int a = 9;
    {
        int a = 7;
        a = 2;
        //return a;
    }
    {}
    ;
    {};
    a = 3;
    return a;
}
