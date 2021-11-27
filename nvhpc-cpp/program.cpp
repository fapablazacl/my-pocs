
#include <iostream>
#include <execution>
#include <vector>
#include <algorithm>
#include <numeric>


struct Employee {
    Employee(const int age) : _age(age) {}

    int age() const {
        return _age;
    }

    int _age = 0;
};


int main() {
    std::vector<Employee> employees = {
        {34}, {31}, {27}, {21}, {57}, {40}, {39}, {21}
    };

    const int ave_age =
        std::transform_reduce(std::execution::par_unseq,
                            employees.begin(), employees.end(),
                            0, std::plus<int>(),
                            [](const Employee& emp){
                                return emp.age();
                            })
        / employees.size();



    std::cout << "The average age is: " << ave_age << std::endl;

    return 0;
}
