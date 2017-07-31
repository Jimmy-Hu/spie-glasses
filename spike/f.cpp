// struct f
// {
//     int operator();
// };
int main()
{
    auto eat = [](auto && ...args) {};
    eat();
    return 0;
}
