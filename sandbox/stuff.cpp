#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#define DEFAULT FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE

#pragma execution_character_set("utf-8")

typedef unsigned int uint;

struct Player {
    char* name;
    double total_time;
    uint hints;
};

void UpdateStats(char*, const uint, const uint, const int);

void setConsoleColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void ThreePoints() {
    // декоративная функция, предназначенная для последовательного вывода трех точек с интервалом во времени
    printf(".");
    Sleep(300);
    printf(".");
    Sleep(300);
    printf(".");
    Sleep(1500);
}


class Cell {
private:
    WORD color;
    const char* type; // 0-8 - usual cell, * - bomb
    const char* status; // 0 - hidden, 1 - revealed, ⁋ - flag, ? - question mark (obviously), ✕ - wrong mine guess
public:
    Cell();
    ~Cell();
    void revealCell();
    bool isHidden();
    void hideCell();
    const char* getType();
    const char* getStatus();
    void setType(const char* type);
    void setStatus(const char* status);
    WORD getColor();
    void setColor(WORD color);
};

Cell::Cell() {

}

Cell::~Cell() {}

bool Cell::isHidden() {
    return strcmp(status, "0") == 0;
}

void Cell::hideCell() {
    this->status = u8"0";
}

void Cell::revealCell() {
    this->status = u8"1";
}

const char* Cell::getType() {
    return type;
}

const char* Cell::getStatus() {
    return status;
}

void Cell::setType(const char* type) {
    this->type = type;
}

void Cell::setStatus(const char* status) {
    this->status = status;
}

WORD Cell::getColor() {
    return color;
}

void Cell::setColor(WORD color) {
    this->color = color;
}

class Field {
private:
    Cell** field;
    uint size, ipos = 0, jpos = 0, victory_condition, opened_cells = 0, hints = 3, total_hints = 3;
    bool loss = false;
    int mines_amount;
private:
    void DefaultStatus();
    void GetMinesAmount();
    void GenerateMines();
    void UpdateCells();
    void RecOpening(uint x, uint y, bool border);
public:
    Field(uint size);
    ~Field();
    void DrawField();
    void MakeMove();
    bool lossCheck();
    bool victoryCheck();
    void lossUpdate();
    void victoryUpdate();
    void removeCursor();
    void useHint();
    uint getHints();
    uint getTotalHints();
};

Field::Field(uint size) {
    this->size = size;
    field = (Cell**)calloc(size, sizeof(Cell*));
    if (field == NULL) {
        puts("Не удалось выделить память, выход из программы...");
        exit(-1);
    }
    for (int i = 0; i < size; ++i) {
        field[i] = (Cell*)calloc(size, sizeof(Cell));
        if (field[i] == NULL) {
            puts("Не удалось выделить память, выход из программы...");
            exit(-1);
        }
    }
    DefaultStatus();
    GetMinesAmount();
    GenerateMines();
    UpdateCells();
}

Field::~Field() {
    for (int i = 0; i < size; ++i) free(field[i]);
    free(field);
}

void Field::DefaultStatus() {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            field[i][j].setType(u8"0");
            field[i][j].hideCell();
        }
    }
}

void Field::GetMinesAmount() {
    switch (size) {
    case 9:
        mines_amount = 10;
        break;
    case 16:
        mines_amount = 40;
        break;
    case 25:
        mines_amount = 112;
        break;
    } 
    victory_condition = size * size - mines_amount;
}

void Field::GenerateMines() {
    int** mines = (int**)calloc(mines_amount, sizeof(int*)), index = 0;
    if (mines == NULL) {
        puts("Не удалось выделить память. Выход из программы...");
        exit(-1);
    }
    for (int i = 0; i < mines_amount; ++i) {
        mines[i] = (int*)calloc(2, sizeof(int));
        if (mines[i] == NULL) {
            puts("Не удалось выделить память. Выход из программы...");
            exit(-1);
        }
    }
    while (1) {
        int x = rand() % size, y = rand() % size, f = 1;
        for (int i = 0; i < index && f; ++i) {
            if (mines[i][0] == x && mines[i][1] == y) f = 0;
        }
        if (f) {
            mines[index][0] = x;
            mines[index++][1] = y;
        }
        if (index == mines_amount) break;
    }

    for (int i = 0; i < mines_amount; ++i) {
        int x = mines[i][0], y = mines[i][1];
        field[x][y].setType(u8"*");
        field[x][y].setColor(FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
    }
    for (int i = 0; i < mines_amount; ++i) free(mines[i]);
    free(mines);
}

void Field::UpdateCells() {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
        	int amount = 0;
        	if (!strcmp(field[i][j].getType(), "*")) continue;
        	else {
        		if (i > 0) amount += strcmp(field[i - 1][j].getType(), "*") == 0;							    // top
        		if (i < size - 1) amount += strcmp(field[i + 1][j].getType(), "*") == 0;					    // bottom
        		if (j < size - 1) amount += strcmp(field[i][j + 1].getType(), "*") == 0;					    // right
        		if (j > 0) amount += strcmp(field[i][j - 1].getType(), "*") == 0;							    // left
        		if (i > 0 && j < size - 1) amount += strcmp(field[i - 1][j + 1].getType(), "*") == 0;		    // top-right
        		if (i < size - 1 && j < size - 1) amount += strcmp(field[i + 1][j + 1].getType(), "*") == 0;  // bottom-right
        		if (i < size - 1 && j > 0) amount += strcmp(field[i + 1][j - 1].getType(), "*") == 0;		    // bottom-left
                if (i > 0 && j > 0) amount += strcmp(field[i - 1][j - 1].getType(), "*") == 0;                // top-left
        	}
            switch (amount) {
            case 0:
                field[i][j].setType(u8"0");
                field[i][j].setColor(FOREGROUND_INTENSITY);
                break;
            case 1:
                field[i][j].setType(u8"1");
                field[i][j].setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                break;
            case 2:
                field[i][j].setType(u8"2");
                field[i][j].setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;
            case 3:
                field[i][j].setType(u8"3");
                field[i][j].setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                break;
            case 4:
                field[i][j].setType(u8"4");
                field[i][j].setColor(FOREGROUND_BLUE);
                break;
            case 5:
                field[i][j].setType(u8"5");
                field[i][j].setColor(FOREGROUND_RED);
                break;
            case 6:
                field[i][j].setType(u8"6");
                field[i][j].setColor(FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            case 7:
                field[i][j].setType(u8"7");
                field[i][j].setColor(FOREGROUND_RED | FOREGROUND_GREEN);
                break;
            case 8:
                field[i][j].setType(u8"8");
                field[i][j].setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                break;
            }
        }
    }
}

void Field::DrawField() {
    char hidden_cell[] = u8"■";
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (strcmp(field[i][j].getStatus(), "1") == 0) {
                if (strcmp(field[i][j].getType(), "*") == 0)
                    setConsoleColor(BACKGROUND_RED);
                else if (i == ipos && j == jpos)
                    setConsoleColor(field[i][j].getColor() | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
                else setConsoleColor(field[i][j].getColor());
                printf("%s", field[i][j].getType());
                setConsoleColor(DEFAULT);
                printf("  ");

        	}
        	else if (strcmp(field[i][j].getStatus(), "0") == 0) {
                if (field[i][j].getType() == "*" && loss) {
                    field[i][j].revealCell();
                    printf("%s", field[i][j].getType());
                }
                else if (i == ipos && j == jpos) {
                    setConsoleColor(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
                    printf("%s", hidden_cell);
                }
                else printf("%s", hidden_cell);
                setConsoleColor(DEFAULT);
                printf("  ");
        	}
            else if (strcmp(field[i][j].getStatus(), "⁋") == 0) {
                if (i == ipos && j == jpos)
                    setConsoleColor(FOREGROUND_RED | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
                else setConsoleColor(FOREGROUND_RED);
                printf("%s", "⁋");
                setConsoleColor(DEFAULT);
                printf("  ");
            }
            else if (field[i][j].getStatus() == "?") {
                if (i == ipos && j == jpos)
                    setConsoleColor(FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
                else setConsoleColor(FOREGROUND_INTENSITY);
                printf("%s", "?");
                setConsoleColor(DEFAULT);
                printf("  ");
            }
            else {
                setConsoleColor(FOREGROUND_RED);
                printf("%s", "✕");
                setConsoleColor(DEFAULT);
                printf("  ");
            }
        }
        printf("\n");
    }
    printf("\nБомб осталось: %d\n", mines_amount);
}

void Field::MakeMove() {
    char move = tolower(_getch());
    if (_kbhit()) {
        char move = _getch();
    }

    if (move == 'a' && jpos > 0) jpos--;
    if (move == 'd' && jpos < size - 1) jpos++;
    if (move == 'w' && ipos > 0) ipos--;
    if (move == 's' && ipos < size - 1) ipos++;

    if (move == 13 && strcmp(field[ipos][jpos].getStatus(), "1") != 0) {
        puts("\n\nВыберите действие с клеткой:");
        puts("1. Открыть клетку");
        puts("2. Пометить клетку флагом");
        puts("3. Пометить клетку вопросительным знаком");
        puts("4. Воспользоваться подсказкой и узнать, что находится на этой клетке");
        puts("5. Ничего не делать");
        printf(">>> ");
        int choice = -1;
        while (scanf("%d", &choice) != 1 || choice < 1 || choice > 5) {
            puts("Проверьте корректность ввода.");
            while (getchar() != '\n');
            printf(">>> ");
        }
        if (choice == 1) {
            if (strcmp(field[ipos][jpos].getType(), "*") == 0) {
                field[ipos][jpos].revealCell();
                loss = true;
            }
            else if (strcmp(field[ipos][jpos].getStatus(), "0") == 0) 
                RecOpening(ipos, jpos, !field[ipos][jpos].isHidden() || strcmp(field[ipos][jpos].getType(), "0") != 0);
            else {
                if (strcmp(field[ipos][jpos].getStatus(), "⁋") == 0) mines_amount++;
                opened_cells++;
                field[ipos][jpos].setStatus(u8"1");
            }
        }
        if (choice == 2) {
            if (field[ipos][jpos].getStatus() != "⁋") mines_amount--;
            field[ipos][jpos].setStatus(u8"⁋");
        }
        if (choice == 3) {
            if (field[ipos][jpos].getStatus() == "⁋") mines_amount++;
            field[ipos][jpos].setStatus(u8"?");
        }
        if (choice == 4) useHint();
    }
}

void Field::RecOpening(uint i, uint j, bool border) {
    if (field[i][j].isHidden()) {
        opened_cells++;
        field[i][j].revealCell();
    }
    if (border) return;
    if (i > 0 && j > 0) 
        RecOpening(i - 1, j - 1, !field[i - 1][j - 1].isHidden() || strcmp(field[i - 1][j - 1].getType(), "0") != 0); // top-left
    if (i > 0) 
        RecOpening(i - 1, j, !field[i - 1][j].isHidden() || strcmp(field[i - 1][j].getType(), "0") != 0);             // top
    if (i > 0 && j < size - 1) 
        RecOpening(i - 1, j + 1, !field[i - 1][j + 1].isHidden() || strcmp(field[i - 1][j + 1].getType(), "0") != 0); // top-right
    if (j < size - 1) 
        RecOpening(i, j + 1, !field[i][j + 1].isHidden() || strcmp(field[i][j + 1].getType(), "0") != 0);             // right
    if (i < size - 1 && j < size - 1) 
        RecOpening(i + 1, j + 1, !field[i + 1][j + 1].isHidden() || strcmp(field[i + 1][j + 1].getType(), "0") != 0); // bottom-right
    if (i < size - 1) 
        RecOpening(i + 1, j, !field[i + 1][j].isHidden() || strcmp(field[i + 1][j].getType(), "0") != 0);             // bottom
    if (i < size - 1 && j > 0) 
        RecOpening(i + 1, j - 1, !field[i + 1][j - 1].isHidden() || strcmp(field[i + 1][j - 1].getType(), "0") != 0); // bottom-left
    if (j > 0) 
        RecOpening(i, j - 1, !field[i][j - 1].isHidden() || strcmp(field[i][j - 1].getType(), "0") != 0);             // left
}

bool Field::lossCheck() {
    return loss;
}

bool Field::victoryCheck() {
    return opened_cells == victory_condition;
}

void Field::lossUpdate() {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (field[i][j].getStatus() == "⁋" && field[i][j].getType() != "*") {
                field[i][j].setStatus(u8"✕");
            }
        }
    }
}

void Field::victoryUpdate() {
    mines_amount = 0;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (field[i][j].getType() == "*" && strcmp(field[i][j].getStatus(), "⁋") != 0) 
                field[i][j].setStatus(u8"⁋");
        }
    }
}

void Field::removeCursor() {
    ipos = jpos = size;
}

void Field::useHint() {
    if (hints) {
        if (field[ipos][jpos].getType() == "*") puts("\nНа данной клетке находится мина!");
        else puts("\nНа данной клетке нет мины.");
        hints--;
        printf("\nОсталось подсказок: %d", hints);
        
    }
    else puts("\nУ вас закончились подсказки.");
    printf("\nНажмите любую клавишу, чтобы продолжить... ");
    _getch();   
}

uint Field::getHints() {
    return hints;
}

uint Field::getTotalHints() {
    return total_hints;
}

int MainMenu() {
    system("cls");
    puts("Добро пожаловать в \"Консольного сапера\"!\n");
    puts("1. Начать игру");
    puts("2. Правила игры");
    puts("3. Статистика лучших игр за все время");
    puts("4. Условные обозначения на поле и управление");
    puts("5. Выход из программы");
    printf("\nВыберите один из пунктов:\n>>> ");
    int choice = -1;
    while (scanf("%d", &choice) != 1 || choice < 1 || choice > 5) {
        puts("Проверьте корректность ввода.");
        while (getchar() != '\n');
        printf(">>> ");
    }
    return choice;
}

int GameMode() {
    system("cls");
    puts(">--- Выбор сложности игры ---<\n");
    puts("1. Новичок (9x9)");
    puts("2. Любитель (16x16)");
    puts("3. Профессионал (25x25)");
    puts("4. Вернуться в главное меню");
    printf("\nВыберите один из пунктов:\n>>> ");
    int mode = -1;
    while (scanf("%d", &mode) != 1 || mode < 1 || mode > 4) {
        puts("Проверьте корректность ввода.");
        while (getchar() != '\n');
        printf(">>> ");
    }
    int size;
    switch (mode) {
    case 1:
        size = 9;
        break;
    case 2:
        size = 16;
        break;
    case 3:
        size = 25;
        break;
    case 4:
        size = 0;
        break;
    }
    return size;
}

void Game() {
    int size = GameMode();
    bool running = size != 0;
    if (running) {
        clock_t start = clock();
        Field field(size);
        while (running) {
            system("cls");
            field.DrawField();
            field.MakeMove();
            if (field.lossCheck()) {
                system("cls");
                field.lossUpdate();
                field.DrawField();
                puts("\nВы проиграли.\n");
                puts("Хотите сыграть еще раз?");
                printf("Чтобы сыграть снова, введите 'Y', в противном случае - любую другую клавишу:\n>>> ");
                char confirm[50];
                scanf("%s", confirm);
                while (getchar() != '\n');

                if (_stricmp(confirm, "y") == 0) {
                    running = false;
                    printf("\nПодождите");
                    ThreePoints();
                    Game();
                }
                else {
                    printf("\nПереход в главное меню, подождите");
                    ThreePoints();
                    running = false;
                }
            }
            if (field.victoryCheck()) {
                int result = (clock() - start) * 1000 / CLOCKS_PER_SEC;
                system("cls");
                field.removeCursor();
                field.victoryUpdate();
                field.DrawField();
                printf("\nВы победили!");
                printf("\nВаше время - %d секунд %d миллисекунд", result / 1000, result % 1000);
                printf("\nВведите ваше имя:\n");
                printf(">>> ");
                char name[50];
                scanf("%s", name); 
                UpdateStats(name, size, field.getTotalHints() - field.getHints(), result);
                puts("\nХотите сыграть еще раз?");
                printf("Чтобы сыграть снова, введите 'Y', в противном случае - любую другую клавишу:\n>>> ");
                char confirm[50];
                scanf("%s", confirm);
                while (getchar() != '\n');

                if (_stricmp(confirm, "y") == 0) {
                    running = false;
                    printf("\nПодождите");
                    ThreePoints();
                    Game();
                }
                else {
                    printf("\nПереход в главное меню, подождите");
                    ThreePoints();
                    running = false;
                }
            }
        }
    }
    else return;
}

void Rules() {
    system("cls");
    puts(">--- Правила игры ---<\n");
    puts("В начале у игрока есть поле, заполненное закрытыми полями. \
Некоторые из этих полей скрывают мины, а \nнекоторые нет. Задача игрока \
- определить под каким полем скрывается мина и помечать\nэти поля. Игрок должен также открыть \
те поля, где нет мин. Если игрок открывает поле с миной,\nон проигрывает. \
Если игрок открывает поле без мины, появляется номер, указывающий, сколько \
мин\nнаходится в восьми соседних полях. Базируясь на этих числах, \
игрок должен определить, где\nнаходятся мины. Цель игры - нахождение \
всех мин, посредством открытия полей, в которых нет мин.\n");
    printf("Нажмите любую клавишу, чтобы перейти в главное меню... ");
    _getch();
}

char* StatsMenu() {
    system("cls");
    puts(">--- Выбор статистики ---<\n");
    puts("Выберите одну из сложностей, по которой будет выведена статистика:");
    puts("1. Новичок");
    puts("2. Любитель");
    puts("3. Профессионал");
    printf(">>> ");
    int choice = -1;;
    while (scanf("%d", &choice) != 1 || choice < 1 || choice > 3) {
        puts("Проверьте корректность ввода.");
        while (getchar() != '\n');
        printf(">>> ");
    }
    char path[30];
    switch (choice) {
    case 1:
        strcpy(path, "stats/beginner_stats.txt");
        break;
    case 2:
        strcpy(path, "stats/amateur_stats.txt");
        break;
    case 3:
        strcpy(path, "stats/professional_stats.txt");
        break;
    }   
    return path;
}

void Stats() {
    system("cls");
    char path[30];
    strcpy(path, StatsMenu());
    FILE* stats = fopen(path, "r");
    system("cls");
    puts(">--- Статистика ---<\n");
    int amount;
    fscanf(stats, "%d ", &amount);
    for (int i = 0; i < amount; ++i) {
        char buff[50];
        fscanf(stats, "%s", buff);
        char* ptr = strtok(buff, ";");
        printf("%d. Имя игрока: %s\n", i + 1, buff);
        ptr = strtok(NULL, ";");
        printf("Время: %.3lf\n", atof(ptr));
        ptr = strtok(NULL, ";");
        printf("Использовано подсказок: %d\n\n", atoi(ptr));
    }
    printf("Нажмите любую клавишу, чтобы продолжить... ");
    _getch();
}

void UpdateStats(char* name, const uint size, const uint hints, const int result) {
    char path[30];
    double total_time = (result / 1000) + (result % 1000 / 1000.);
    switch (size) {
    case 9:
        strcpy(path, "stats/beginner_stats.txt");
        break;
    case 16:
        strcpy(path, "stats/amateur_stats.txt");
        break;
    case 25:
        strcpy(path, "stats/professional_stats.txt");
        break;
    }

    FILE* stats = fopen(path, "r");
    FILE* temp = fopen("stats/temp.txt", "w");
    if (stats == NULL || temp == NULL) {
        puts("Не удалось открыть файл, выход из программы...");
        exit(-2);
    }

    int amount;
    fscanf(stats, "%d ", &amount);

    Player* players, *temp_players;
 
    players = (Player*)calloc(amount + 1, sizeof(Player));
    temp_players = (Player*)calloc(amount + 1, sizeof(Player));
  
    if (players == NULL || temp_players == NULL) {
        puts("Не удалось выделить память, выход из программы...");
        exit(-1);
    }

    for (int i = 0; i < amount; ++i) {
        players[i].name = (char*)calloc(50, sizeof(char));
        char buff[100], *ptr;
        fscanf(stats, "%s", buff);
        ptr = strtok(buff, ";");
        strcpy(players[i].name, ptr);
        ptr = strtok(NULL, ";");
        players[i].total_time = atof(ptr);
        ptr = strtok(NULL, ";");
        players[i].hints = atoi(ptr);
    }
    Player current_result = { name, total_time, hints };
    bool f = true;
    for (int i = 0, k = 0; i < amount; ++i, ++k) {
        if (players[i].total_time >= current_result.total_time && f) {
            temp_players[k++] = current_result;
            f = false;
        }
        temp_players[k] = players[i];
    }
    if (f) temp_players[amount] = current_result;
    
    if (amount < 10) {
        fprintf(temp, "%d\n", amount + 1);
        for (int i = 0; i <= amount; ++i) {
            fprintf(temp, "%s;%lf;%d\n", temp_players[i].name, temp_players[i].total_time, temp_players[i].hints);
        }
    }
    else {
        fprintf(temp, "%d\n", 10);
        for (int i = 0; i < 10; ++i) {
            fprintf(temp, "%s;%lf;%d\n", temp_players[i].name, temp_players[i].total_time, temp_players[i].hints);
        }
    }
    
    fclose(stats);
    fclose(temp);
    remove(path);
    rename("stats/temp.txt", path);
    
    for (int i = 0; i < amount; ++i) free(players[i].name);
    free(players);
    free(temp_players);
}

void DesignationAndControls() {
    system("cls");
    puts(">--- Обозначения и управление ---<\n");
    setConsoleColor(FOREGROUND_INTENSITY);
    printf("%s ", "0");
    setConsoleColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    printf("%s ", "1");
    setConsoleColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("%s ", "2");
    setConsoleColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf("%s ", "3");
    setConsoleColor(FOREGROUND_BLUE);;
    printf("%s ", "4");
    setConsoleColor(FOREGROUND_RED);
    printf("%s ", "5");
    setConsoleColor(FOREGROUND_GREEN | FOREGROUND_BLUE);
    printf("%s ", "6");
    setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN);
    printf("%s ", "7");
    setConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("%s ", "8");
    setConsoleColor(DEFAULT);
    puts("- количество мин относительно данной клетки");
    printf("%s - неоткрытая клетка\n", "■");
    puts("* - бомба (такая клетка появится, если при проигрыше остались еще неотмеченные мины)");
    setConsoleColor(FOREGROUND_RED);
    printf("%s", "⁋");
    setConsoleColor(DEFAULT);
    puts(" - флажок (пользовательская отметка, на которой находится предполагаемая мина)");
    setConsoleColor(FOREGROUND_INTENSITY);
    printf("?");
    setConsoleColor(DEFAULT);
    puts(" - вопросительный знак (пользовательская отметка, о статусе которой пользователь сомневается)");
    setConsoleColor(BACKGROUND_RED);
    printf("*");
    setConsoleColor(DEFAULT);
    puts(" - взорванная мина");
    setConsoleColor(FOREGROUND_RED);
    printf("%s", "✕");
    setConsoleColor(DEFAULT);
    puts(" - неверно отмеченная мина (появляется при проигрыше)\n");
    puts("Управление: WASD - управление курсором (убедитесь в том, что играете с включенной английской раскладкой)\n\t    ENTER - взаимодействие с выбранной клеткой\n");
    printf("Нажмите любую клавишу, чтобы перейти в главное меню... ");
    _getch();
}

int main() {
    SetConsoleTitleA("Console minesweeper (why)");
    SetConsoleOutputCP(CP_UTF8);
    srand(time(NULL));

    bool running = true;
    while (running) {
        system("cls");
        int choice = MainMenu();
        if (choice == 1) Game();
        if (choice == 2) Rules();
        if (choice == 3) Stats();
        if (choice == 4) DesignationAndControls();
        if (choice == 5) running = false;
    }
    
    return 0;
}
