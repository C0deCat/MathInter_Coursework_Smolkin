#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "MathCore/Interpreter.h"
#include "MathCore/Errors.h"

static void enableUtf8Console() {
    // Для UTF-8 в консоли Windows:
    // - cp 65001 (UTF-8)
    // - проект собран с /utf-8
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
}

static void printHelp() {
    std::cout
        << "Команды:\n"
        << "  помощь               - показать справку\n"
        << "  выход                - завершить\n"
        << "  файл <путь>          - выполнить команды из файла\n"
        << "Синтаксис:\n"
        << "  X = выражение\n"
        << "  выражение\n"
        << "Примеры:\n"
        << "  V1 = [ 1 2 3 ]\n"
        << "  M1 = [ 1 0 0; 0 1 0; 0 0 1 ]\n"
        << "  V2 = M1 * V1\n"
        << "  R = 1 / 3\n"
        << "  V3 = V2 * R\n"
        << "  M2 = T(M1)\n"
        << "  V3\n"
        << "  M2\n";
}

static void executeFile(mathcore::Interpreter& interp, const std::filesystem::path& p) {
    if (!std::filesystem::exists(p)) {
        std::cout << "Ошибка: файл не найден: " << p.u8string() << "\n";
        return;
    }

    std::ifstream in(p);
    if (!in) {
        std::cout << "Ошибка: не удалось открыть файл: " << p.u8string() << "\n";
        return;
    }

    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        if (line.empty()) continue;

        try {
            auto res = interp.executeLine(line);
            if (res && *res) std::cout << (*res)->toString() << "\n";
        }
        catch (const mathcore::ParseError& e) {
            std::cout << "Синтаксическая ошибка (строка " << lineNo << ", позиция " << e.col << "): " << e.what() << "\n";
        }
        catch (const mathcore::EvalError& e) {
            std::cout << "Ошибка вычисления (строка " << lineNo << "): " << e.what() << "\n";
        }
        catch (const std::exception& e) {
            std::cout << "Неизвестная ошибка (строка " << lineNo << "): " << e.what() << "\n";
        }
    }
}

static std::string trimCmd(std::string s) {
    auto is_ws = [](unsigned char ch) { return std::isspace(ch) != 0; };
    while (!s.empty() && is_ws(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && is_ws(static_cast<unsigned char>(s.back())))  s.pop_back();
    // На случай '\r' (иногда попадает в конец строки)
    if (!s.empty() && s.back() == '\r') s.pop_back();
    return s;
}

int main(int argc, char** argv) {
    enableUtf8Console();

    mathcore::Interpreter interp;

    // Режим файла: MathCLI.exe <filePath>
    if (argc >= 2) {
        executeFile(interp, std::filesystem::path(argv[1]));
        return 0;
    }

    std::cout << "Математический интерпретатор (введите 'помощь' для справки)\n";

    while (true) {
        std::cout << ">>> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;

        if (line == "выход") break;
        if (line == "помощь") { printHelp(); continue; }

        // команда: файл <путь>
        const std::string cmdFile = u8"файл ";
        if (line.rfind(cmdFile, 0) == 0) {
            std::string raw = trimCmd(line.substr(cmdFile.size()));

            // Поддержка пути в кавычках: файл "D:\...\example.txt"
            if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') {
                raw = raw.substr(1, raw.size() - 2);
            }

            auto path = std::filesystem::u8path(raw);
            executeFile(interp, path);
            continue;
        }

        try {
            auto res = interp.executeLine(line);
            if (res && *res) std::cout << (*res)->toString() << "\n";
        }
        catch (const mathcore::ParseError& e) {
            std::cout << "Синтаксическая ошибка (позиция " << e.col << "): " << e.what() << "\n";
        }
        catch (const mathcore::EvalError& e) {
            std::cout << "Ошибка вычисления: " << e.what() << "\n";
        }
        catch (const std::exception& e) {
            std::cout << "Неизвестная ошибка: " << e.what() << "\n";
        }
    }

    return 0;
}
