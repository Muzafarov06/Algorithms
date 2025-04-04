#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cctype>     // Для isdigit()
#include <algorithm>  // Для all_of()
#include "stack.h"

using namespace std;

const int RETURN_MARKER = -2; // Специальный маркер для call/ret

class CPU {
private:
    Stack* stack;
    unordered_map<string, int> registers;
    bool error = false;

public:
    CPU() {
        stack = stack_create();
    }

    ~CPU() {
        stack_delete(stack);
    }

    // Проверка, является ли строка числом (включая отрицательные)
    bool isNumber(const string& str) {
        if (str.empty()) return false;
        int value = 0;
        try {
            size_t idx;
            value = stoi(str, &idx);
            if (idx != str.size()) return false; // проверяем, что вся строка — число
        } catch (...) {
            return false;
        }
        if (value == RETURN_MARKER) return false;
        return true;
    }

    void push(const string& operand) {
        if (isNumber(operand)) {
            int value = stoi(operand);
            if (value == RETURN_MARKER) {
                cout << "ERROR: Invalid push value (-2 is reserved)" << endl;
                error = true;
                return;
            }
            stack_push(stack, value);
        } else {
            stack_push(stack, registers[operand]);
        }
    }

    void pop(const string& reg) {
        if (stack_empty(stack)) {
            cout << "ERROR: Stack underflow" << endl;
            error = true;
            return;
        }
        registers[reg] = stack_get(stack);
        stack_pop(stack);
    }

    void add() { operate('+'); }
    void sub() { operate('-'); }
    void mul() { operate('*'); }

    void call(int returnAddress) {
        stack_push(stack, RETURN_MARKER); // Добавляем маркер перед адресом возврата
        stack_push(stack, returnAddress);
    }

    int ret() {
        if (stack_empty(stack)) {
            cout << "ERROR: Nothing to return to" << endl;
            error = true;
            return -1;
        }

        int value = stack_get(stack);
        stack_pop(stack);

        if (value != RETURN_MARKER) {
            cout << "ERROR: Invalid return sequence" << endl;
            error = true;
            return -1;
        }

        if (stack_empty(stack)) {
            cout << "ERROR: Stack corrupted, missing return address" << endl;
            error = true;
            return -1;
        }

        int returnAddress = stack_get(stack);
        stack_pop(stack);
        return returnAddress;
    }

    void print_registers() {
        if (!error) {
            for (const auto& reg : registers) {
                cout << reg.first << " = " << reg.second << endl;
            }
        }
    }

private:
    void operate(char op) {
        if (stack_size(stack) < 2) {
            cout << "ERROR: Stack underflow" << endl;
            error = true;
            return;
        }
        int b = stack_get(stack);
        stack_pop(stack);
        int a = stack_get(stack);
        stack_pop(stack);

        switch (op) {
            case '+': stack_push(stack, a + b); break;
            case '-': stack_push(stack, a - b); break;
            case '*': stack_push(stack, a * b); break;
        }
    }
};

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    istream* input = &cin;
    ifstream inputFile;

    if (argc >= 2) {
        inputFile.open(argv[1]);
        if (!inputFile) {
            cerr << "Ошибка: не удалось открыть файл " << argv[1] << endl;
            return 1;
        }
        input = &inputFile;
    }

    CPU cpu;
    string command;
    int programCounter = 0;
    unordered_map<int, int> callStack;

    while (*input >> command) {
        programCounter++;

        if (command == "push") {
            string operand;
            *input >> operand;
            cpu.push(operand);
        } else if (command == "pop") {
            string reg;
            *input >> reg;
            cpu.pop(reg);
        } else if (command == "add") {
            cpu.add();
        } else if (command == "sub") {
            cpu.sub();
        } else if (command == "mul") {
            cpu.mul();
        } else if (command == "call") {
            callStack[programCounter] = programCounter;
            cpu.call(programCounter);
        } else if (command == "ret") {
            int returnAddress = cpu.ret();
            if (returnAddress != -1) {
                programCounter = returnAddress;
            }
        } else {
            cout << "ERROR: Unknown command " << command << endl;
            return 1;
        }
    }

    cpu.print_registers();
    return 0;
}
