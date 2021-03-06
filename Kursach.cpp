#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <malloc.h>
#include <clocale>
#include <time.h>
#include <windows.h>
#include <assert.h>

//Программа не позволит пользователю добавить больше 23 ЯП и не позволит добавлять ЯП с одинаковыми именами. Но всё же возможно залезть в DATABASE и сделать это вручную, делать так не надо:)
//Я мог реализовать и другой вариант, где при переполнении программа просто добавляет новые страницы в консоли, а пользователь переключается между ними стрелочками, выходит кнопкой ESC
//Но новые идеи будут приходить и приходить, поэтому остановлюсь на этом.
//Новые знания в этой курсовой - познакомился с некоторыми функциями библиотеки для win, не знал, что getch() работает похоже на getchar(), но просто не выводит принятый в консоли символ на экран
//Я думал, getch() считывает события клавиатуры, но можно убедиться в обратном, если просто нажать кнопки вне консоли (к примеру, если прикрыл консоль и печатаю что-то не в неё)

using namespace std;
using namespace System;
using namespace System::IO;

/* Предлагаемый вариант для файла DATABASE
14
Elixir 1 1 29 150000 2012 1 0 0 0
PHP 0 0 68 125000 1995 1 0 1 0
Golang 1 1 78 180000 2009 1 1 1 0
C# 1 1 82 135000 2000 1 1 1 1
Python 0 0 100 130000 1991 1 1 1 1
Kotlin 1 1 59 160000 2011 0 1 0 0
Java 1 1 95 150000 1995 1 1 0 1
Ruby 0 0 64 178000 1995 1 1 0 1
Javascript 0 0 88 150000 1995 1 0 0 0
Objective-C 1 0 44 200000 1984 0 1 1 1
Delphi 1 1 38 110000 1986 1 1 0 1
C++ 1 1 92 130000 1985 1 0 1 1
Swift 1 1 70 175000 2014 0 1 1 1
C 1 1 95 100000 1972 1 0 1 1
*/

//коды клавиш
#define ENTER 13
#define ESC 27
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77
#define HOME 71
#define END 79
#define CURSOR_KEYS 224 //224 - для курсорных клавиш (в char будет -32, так как первый бит - "+/-"). 0 - для функциональных (клавиши на цифровой клавиатуре + NumLock). Чтобы не перепутать с другими клавишами на клавиатуре
#define BACKSPACE 8

unsigned short PL_SIZE = 0; //количество ЯП в файле. Использую глобальную переменную, чтобы постоянно не передавать её в функции (всего 2 байта - так что не критично)
const unsigned short MM_SIZE = 16; //кол-во строк в главном меню

System::ConsoleColor bGround = ConsoleColor::Blue; //цвет фона меню
System::ConsoleColor fGround = ConsoleColor::Black; //цвет текста меню
System::ConsoleColor bCGround = ConsoleColor::White; //цвет фона для выделенной (текущей - current) строки меню
System::ConsoleColor fNIGround = ConsoleColor::Green; //цвет текста в том пункте, на который нельзя навестись (к примеру надпись "Главное меню") (NI - non-interactive)
System::ConsoleColor bNIGround = ConsoleColor::DarkBlue; //цвет фона в том пункте, на который нельзя навестись (к примеру надпись "Главное меню") (NI - non-interactive)

struct appArea //область применения ЯП
{
	bool webDev; //веб разработка
	bool mobileDev; //разработка мобильных приложений
	bool gameDev; //разработка компьютерных игр
	bool desktopDev; //разработка десктоп приложений
}; //true - применияется, false - не применяется

struct PL //PL - ЯП (Язык программирования)
{
	char name[15]; //название ЯП
	bool isCompiled; //true - компилируемый, false - интерпретируемый
	bool isStatic; //true - статически типизированный, false - динамически
	unsigned short popularity; //баллы популярности (не больше 100, но больше 0)
	unsigned avgSalary; //средняя ЗП по ЯП (не меньше 50к и не больше 250к (либо расширять консоль, чтобы диаграмма помещалась))
	unsigned short appeared; //год появления ЯП
	appArea area; //область применения ЯП
};

struct Node //двусвязный список, хранит 3 указателя на структуру PL (два - для доступа к след/пред элементам списка, а один - для самого значения узла списка)
{
	Node* prev;
	PL* data; 
	Node* next;
};

char mainMenu[MM_SIZE][80] =
{
	"          Вывести диаграммы для средних зарплат каждого ЯП                     ",
	"     ВЫВОД ЯЗЫКОВ ПРОГРАММИРОВАНИЯ В СЛЕДУЮЩЕМ ПОРЯДКЕ:                        ",
	"          В алфавитном поряке                                                  ",
	"          По убыванию средних зарплат                                          ",
	"          По убыванию популярности                                             ",
	"          Компилируемые в первую очередь                                       ",
	"          Со статической типизацией в первую очередь                           ",
	"          Сперва старые                                                        ",
	"     ВЫВОД ЯЗЫКОВ ПРОГРАММИРОВАНИЯ ДЛЯ КОНКРЕТНОЙ ОБЛАСТИ ПРИМЕНЕНИЯ:          ",
	"          Веб разработка                                                       ",
	"          Мобильная разработка                                                 ",
	"          Разработка игр                                                       ",
	"          Разработка настольных приложений                                     ",
	"     ИЗМЕНЕНИЕ ФАЙЛА С ДАННЫМИ:                                                ",
	"          Добавить ЯП в файл                                                   ",
	"          Удалить ЯП из файла                                                  "
};

//описания(объявления) функций:
unsigned short menu(); //даёт возможность пользователю выбирать конкретный элемент меню
void scanFromFile(PL **pl); //функция записывающая/перезаписывающая данные из файла в массив ЯП
void save2File(PL *pl); //функция сохраняет новый ЯП в файл
void delFromFile(int y); //удаляет нужную строку из файла
void printPLarray(const PL *pl); //выводит полную информацию о всех ЯП в динамическом массиве
void printSalaryDiagram(const PL *pl); //вывод диаграмм для средних ЗП по ЯП
void printPLlist(const Node *current); //выводит полную информацию о всех ЯП в списке
void printRevPLlist(const Node *current); //выводит полную информацию о всех ЯП в списке в обратном порядке
void printAlphabetOrder(const PL *pl); //вывод в алфавитном порядке (здесь покажу пример использования двусвязного списка)
void printSalaryOrder(const PL *pl); //вывод в порядке убывания ЗП (в этой функции создается список по возрастанию ЗП, потом с конца выводится этот список)
void printPopularityOrder(const PL *pl); //вывод в порядке убывания популярности (тот же алгоритм, что и у printSalaryOrder)
void printCompiledFirst(const PL *pl); //выводит сперва компилируемые потом транслируемые ЯП
void printStaticFirst(const PL *pl); //выводит сперва статически типизируемые ЯП
void printOldFirst(const PL *pl); //выводит сперва более старые ЯП (используется двусвязный список)
void printPLappArea(const PL *pl, int appArea); //выводит только языки для конкретной области разработки (appArea: 1 - веб, 2 - мобильная, 3 - игры, 4 - десктоп)
bool addPL(PL *p2pl); //функция будет добавлять ЯП в исходный файл (пользователь будет вводить данные о ЯП). Вёрнет правду, если добавление прошло успешно
bool remPL(PL *pl); //функция будет удалять ЯП из исходного файла (пользователь выберет какой ЯП удалить). Вёрнет правду, если удаление прошло успешно
bool comparePL(const PL *pl, const char name[15]); //сравнивает предполагаемое имя нового ЯП с остальными. Вернёт 1, если ЯП с таким именем уже есть

int main()
{
	PL *pl = NULL; //указатель на структуру PL
	if (!setlocale(LC_CTYPE, "Russian")) //в случае ошибки функция setlocale возвращает нулевой указатель
		return -1;
	Console::CursorVisible::set(false); //отключаем видимость курсосра
	Console::WindowWidth = 120; //устанавливаем ширину консольного окна (в случае недопустимого значения выбросит необработанное исключение, а необработанные исключения приводят к диалоговому окно со сбоем)
	Console::WindowHeight = 30;  //устанавливаем высоту консольного окна (хотя на разных ОС поведение с необработанными исключениями разное: где-то просто сбой, где-то сообщение об ошибке)
	Console::BufferHeight = Console::WindowHeight; //установка размера буфера по ширине
	Console::BufferWidth = Console::WindowWidth; //установка размера буфера по высоте
	scanFromFile(&pl);
	printPLarray(pl);
	getch();
	while(true) //чтобы пользователь мог завершить программу, ему надо нажать esc, в функции menu() программа это обработает и использует функцию exit(0)
	{
		Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
		Console::Clear();
		Console::CursorLeft = 10;
		Console::CursorTop = 4;
		Console::ForegroundColor = fGround; //выбираем цвет текста
		Console::BackgroundColor = bGround; //выбираем цвет фона
		for (int i = 0; i < MM_SIZE; ++i) //выводим главное меню
		{
			if (i == 1 || i == 8 || i == 13)
			{
				Console::ForegroundColor = fNIGround; //выбираем цвет текста для неинтерактивного поля
				Console::BackgroundColor = bNIGround; //выбираем цвет фона для неинтерактивного поля
			}
			Console::CursorLeft = 10;
			Console::CursorTop = 5 + i;
			printf("%s", mainMenu[i]);
			if (i == 1 || i == 8 || i == 13)
			{
				Console::ForegroundColor = fGround; //выбираем цвет текста
				Console::BackgroundColor = bGround; //выбираем цвет фона
			}
		}
		Console::CursorLeft = 10;
		Console::CursorTop = 5 + MM_SIZE;
		unsigned short n = menu(); //меню пользователя
		switch (n)
		{
			case 0:
				printSalaryDiagram(pl);
				getch();
				break;
			case 2:
				printAlphabetOrder(pl);
				getch();
				break;
			case 3:
				printSalaryOrder(pl);
				getch();
				break;
			case 4:
				printPopularityOrder(pl);
				getch();
				break;
			case 5:
				printCompiledFirst(pl);
				getch();
				break;
			case 6:
				printStaticFirst(pl);
				getch();
				break;
			case 7:
				printOldFirst(pl);
				getch();
				break;
			case 9:
				printPLappArea(pl, 1);
				getch();
				break;
			case 10:
				printPLappArea(pl, 2);
				getch();
				break;
			case 11:
				printPLappArea(pl, 3);
				getch();
				break;
			case 12:
				printPLappArea(pl, 4);
				getch();
				break;
			case 14:
				if (addPL(pl))
					scanFromFile(&pl);
				break;
			case 15:
				if (remPL(pl))
					scanFromFile(&pl);
				break;
		}
	}
	return 0;
}

unsigned short menu() //меню пользователя с возможностью выбора
{
	unsigned short y = 0; //текущий пункт меню
	Console::ForegroundColor = fGround;
	Console::BackgroundColor = bCGround; //цвет фона текущей строки (на которой сейчас стоит "курсор")
	Console::CursorLeft = 10;
	Console::CursorTop = y + 5;
	printf("%s", mainMenu[y]);
	while (true)
	{
		switch (getch())
		{
		case CURSOR_KEYS: //чтобы работали только стрелки (без 'P', 'H' и функциональных клавиш)
			switch (getch())
			{
			case DOWN:
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", mainMenu[y]);
				if (y == 0 || y == 7 || y == 12)
					y += 2;
				else
					y == MM_SIZE - 1 ? y = 0 : y++;
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bCGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", mainMenu[y]);
				break;
			case UP:
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", mainMenu[y]);
				if (y == 2 || y == 9 || y == 14)
					y -= 2;
				else
					y == 0 ? y = MM_SIZE - 1 : y--;
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bCGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", mainMenu[y]);
				break;
			case HOME:
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", mainMenu[y]);
				y = 0;
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bCGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", mainMenu[y]);
				break;
			case END:
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", mainMenu[y]);
				y = MM_SIZE - 1;
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bCGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", mainMenu[y]);
				break;
			}
			break;
		case ENTER:
			if (!((y == 14 && PL_SIZE == 23) || (y == 15 && PL_SIZE == 0))) //если ЯП уже максимальное кол-во или их нет - мы не можем добавить или удалить ЯП соответственно
				return y;
			break;
		case ESC:
			exit(0); //завершение программы
		}
	}
 }

void scanFromFile(PL **pl) //функция записывающая данные из файла в массив ЯП
{
	FILE *file; //объявляем указатель на FILE
	file = fopen("DATABASE", "r"); //пытаемся открыть файл DATABASE на чтение, если не удается - функция fopen возвращает нулевой указатель
	assert(file && "файл DATABASE не открыт"); //если не смогли открыть файл - вывод сообщения об ошибке и завершение программы + имя файла и строка
	unsigned short lastSize = PL_SIZE;
	fscanf(file, "%d", &PL_SIZE); //количество ЯП в файле
	if (PL_SIZE != lastSize) //надо ли изменять размер массива
	{
		free(pl[0]); //освобождаю память, чтобы избежать утечки. Вообще - "pl[0]" равноценно "*pl", но переписывать уже не хочу
		pl[0] = (PL*)malloc(PL_SIZE * sizeof(PL)); //выделяем динамическую память под хранение данных о ЯП (инициализвация нулями не нужна, поэтому не calloc)
		for (int i = 0; i < PL_SIZE; ++i) //использую префиксный инкремент, так как он немного быстрее (хотя некоторые компиляторы сами оптимизируют постфиксный в таких ситуациях)
		{
			short isCompiled, isStatic, webDev, mobileDev, gameDev, desktopDev;
			fscanf(file, "%s%hi%hi%hu%u%hu%hi%hi%hi%hi", pl[0][i].name, &isCompiled, &isStatic, &pl[0][i].popularity, &pl[0][i].avgSalary, &pl[0][i].appeared, &webDev, &mobileDev, &gameDev, &desktopDev);
			pl[0][i].isCompiled = isCompiled != 0; pl[0][i].isStatic = isStatic != 0; pl[0][i].area.webDev = webDev != 0;
			pl[0][i].area.mobileDev = mobileDev != 0; pl[0][i].area.gameDev = gameDev != 0; pl[0][i].area.desktopDev = desktopDev != 0; 
		}
		//так как для bool нет спецификатора - буду читать в short, после чего проверять, не равно ли нулю. На c-style cast к bool даёт warning C4800 -_-
	}
	else //если размер не изменился - нет смысла в лишних операциях по освобождению и выделению динамической памяти
		for (int i = 0; i < PL_SIZE; ++i) //использую префиксный инкремент, так как он немного быстрее (хотя некоторые компиляторы сами оптимизируют постфиксный в таких ситуациях)
		{
			short isCompiled, isStatic, webDev, mobileDev, gameDev, desktopDev;
			fscanf(file, "%s%hi%hi%hu%u%hu%hi%hi%hi%hi", pl[0][i].name, &isCompiled, &isStatic, &pl[0][i].popularity, &pl[0][i].avgSalary, &pl[0][i].appeared, &webDev, &mobileDev, &gameDev, &desktopDev);
			pl[0][i].isCompiled = isCompiled != 0; pl[0][i].isStatic = isStatic != 0; pl[0][i].area.webDev = webDev != 0;
			pl[0][i].area.mobileDev = mobileDev != 0; pl[0][i].area.gameDev = gameDev != 0; pl[0][i].area.desktopDev = desktopDev != 0; 
		} //так как для bool нет спецификатора - буду читать в short, после чего использовать c-style cast в bool
		fclose(file);
		if (PL_SIZE == 23) strcpy(mainMenu[14], "          Добавить ЯП в файл                               (НЕВОЗМОЖНО)        "); //случаЙ, когда больше нельзя добавить ЯП
		else strcpy(mainMenu[14], "          Добавить ЯП в файл                                                   ");
		if (PL_SIZE == 0) strcpy(mainMenu[15], "          Удалить ЯП из файла                              (НЕВОЗМОЖНО)        "); //случай, когда ЯП нет
		else strcpy(mainMenu[15], "          Удалить ЯП из файла                                                  ");
}

void save2File(PL *pl)
{
	FILE *file;
	file = fopen("DATABASE", "r+"); //открываем на чтение и запись
	assert(file && "файл DATABASE не открыт");
	unsigned short size;
	fscanf(file, "%d", &size); //получаем кол-во ЯП в файле
	char chSize[3];
	itoa(size + 1, chSize, 10);
	fseek(file, 0, SEEK_SET); //курсор в файле ставим на начало
	fprintf(file, "%s", chSize); //увеличиваем запись о кол-ве ЯП в файле
	fseek(file, 0, SEEK_END); //курсор в файле ставим на конец
	fprintf(file, "%s %hi %hi %hu %u %hu %hi %hi %hi %hi\n", pl->name, pl->isCompiled, pl->isStatic, pl->popularity, pl->avgSalary, pl->appeared, pl->area.webDev, pl->area.mobileDev, pl->area.gameDev, pl->area.desktopDev);
	fclose(file); //это очень важно! а то закроется и сохранится файл только при завершении программы, если убрать эту строку - будет нехорошо:)
}

void delFromFile(int y, PL *pl)
{
	FILE *file;
	file = fopen("DATABASE", "w"); //открываем на запись (перезаписываем файл с нуля)
	assert(file && "файл DATABASE не открыт");
	fprintf(file, "%d\n", PL_SIZE - 1);
	for (int i = 0; i < PL_SIZE; ++i)
		if (i != y)
			fprintf(file, "%s %hi %hi %hu %u %hu %hi %hi %hi %hi\n", pl[i].name, pl[i].isCompiled, pl[i].isStatic, pl[i].popularity,
			pl[i].avgSalary, pl[i].appeared, pl[i].area.webDev, pl[i].area.mobileDev, pl[i].area.gameDev, pl[i].area.desktopDev);
	fclose(file);
}
void printPLarray(const PL *pl) //выводит полную информацию о всех ЯП в динамическом массиве
{
	Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
	Console::Clear();
	Console::ForegroundColor = ConsoleColor::White; //выбираем цвет текста
	printf("               |                  |                  |Популярность|Средняя|Появился|  Web  |Mobile | Game  |Desktop\n");
	printf("      ЯП       |    Транслятор    |    Типизация     |  (0-100)   |ЗП(руб)| (Год)  |Develop|Develop|Develop|Develop\n");
	printf("===============+==================+==================+============+=======+========+=======+=======+=======+=======\n");
	for (int i = 0; i < PL_SIZE; ++i)
	{
		printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[i].name, pl[i].isCompiled?"Компилятор":"Интерпретатор", pl[i].isStatic?"Статическая":"Динамическая", pl[i].popularity);
		printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
		printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[i].area.webDev?'+':'-', pl[i].area.mobileDev?'+':'-', pl[i].area.gameDev?'+':'-', pl[i].area.desktopDev?'+':'-');
	}
}

void printSalaryDiagram(const PL *pl) //вывод диаграмм для средних ЗП по ЯП
{
	System::ConsoleColor Color = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::Black;
	Console::Clear();
	Console::BackgroundColor = bGround;
	Console::ForegroundColor = fGround;
	Console::CursorLeft = 10;
	Console::CursorTop = 4;
	printf("      ЯП       |Средняя ЗП|                                                    ");
	Console::CursorLeft = 10;
	Console::CursorTop = 5;
	printf("===============+==========+====================================================");
	for (int i = 0; i < PL_SIZE; ++i)
	{
		Console::CursorLeft = 10;
		Console::CursorTop = 6 + i;
		printf("%-15s|", pl[i].name);
		printf("%-10u|", pl[i].avgSalary);
		if (Color != System::ConsoleColor::White) //цветов всего 16, синий уже занят фоном, поэтому остается 15, если надо вывести >15 ЯП, то используем старый цвета заново
			Console::BackgroundColor = ++Color;
		else
			Console::BackgroundColor = Color = System::ConsoleColor::Black;
		if (Console::BackgroundColor == bGround) //так как фон синий, цвет шкалы не может быть синий
			Console::BackgroundColor = ++Color;
		for (unsigned int j = 0; j < pl[i].avgSalary / 5000; ++j) 
			printf(" ");
		Console::BackgroundColor = bGround;
		Console::ForegroundColor = fGround;
		for (unsigned int j = 0; j < 52 - pl[i].avgSalary / 5000; ++j)
			printf(" ");
	}
}

void printPLlist(const Node *current) //выводит полную информацию о всех ЯП в списке по порядку (мог перегрузить функцию printPLarray, но имя функции бы не совсем подходило)
{
	while (current->prev != NULL)
		current = current->prev;
	Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
	Console::Clear();
	Console::ForegroundColor = ConsoleColor::White; //выбираем цвет текста
	printf("               |                  |                  |Популярность|Средняя|Появился|  Web  |Mobile | Game  |Desktop\n");
	printf("      ЯП       |    Транслятор    |    Типизация     |  (0-100)   |ЗП(руб)| (Год)  |Develop|Develop|Develop|Develop\n");
	printf("===============+==================+==================+============+=======+========+=======+=======+=======+=======\n");
	while (current) //false - если указатель current нулевой
	{
		printf("%-15s|  %-16s|   %-15s|    %-8hu|", current->data->name, !current->data->isCompiled?"Компилятор":"Интерпретатор", current->data->isStatic?"Статическая":"Динамическая", current->data->popularity);
		printf("%-7u|  %-6hu|", current->data->avgSalary, current->data->appeared);
		printf("   %-4c|   %-4c|   %-4c|   %-4c\n", current->data->area.webDev?'+':'-', current->data->area.mobileDev?'+':'-', current->data->area.gameDev?'+':'-', current->data->area.desktopDev?'+':'-');
		current = current->next;
	}
}

void printRevPLlist(const Node *current) //выводит полную информацию о всех ЯП в списке в обратном порядке(мог перегрузить функцию printPLarray, но имя функции бы не совсем подходило)
{
	while (current->next != NULL)
		current = current->next;
	Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
	Console::Clear();
	Console::ForegroundColor = ConsoleColor::White; //выбираем цвет текста
	printf("               |                  |                  |Популярность|Средняя|Появился|  Web  |Mobile | Game  |Desktop\n");
	printf("      ЯП       |    Транслятор    |    Типизация     |  (0-100)   |ЗП(руб)| (Год)  |Develop|Develop|Develop|Develop\n");
	printf("===============+==================+==================+============+=======+========+=======+=======+=======+=======\n");
	while (current) //false - если указатель current нулевой
	{
		printf("%-15s|  %-16s|   %-15s|    %-8hu|", current->data->name, !current->data->isCompiled?"Компилятор":"Интерпретатор", current->data->isStatic?"Статическая":"Динамическая", current->data->popularity);
		printf("%-7u|  %-6hu|", current->data->avgSalary, current->data->appeared);
		printf("   %-4c|   %-4c|   %-4c|   %-4c\n", current->data->area.webDev?'+':'-', current->data->area.mobileDev?'+':'-', current->data->area.gameDev?'+':'-', current->data->area.desktopDev?'+':'-');
		current = current->prev;
	}
}

void printAlphabetOrder(const PL *pl) //вывод в алфавитном порядке (порядок по номеру в ascii) (здесь покажу пример использования двусвязного списка)
{
	Node *listPL = (Node*)malloc(sizeof(Node)); //указатель на начало списка
	listPL->data = NULL;
	listPL->prev = NULL; //тк первый элемент списка
	Node *current = listPL;
	current->data = (PL*)pl; //"pl" - указатель на массив (т.е. указатель на первый элемент массива)
	for (int j = 1; j < PL_SIZE; ++j) //сравниваем имена. Итератор j - номер элемент в массиве
	{
		for (int k = 0; k < 15; ++k) //сравниваем два имени. Итератор k - символ в имени
			if (pl[j].name[k] < current->data->name[k]) //у каждого символа char есть свой номер в ascii таблице, сравниваем именно эти номера
			{
				current->data = (PL*)&pl[j];
				break;
			}
	}
	//далее идёт алгоритм выделения памяти для узлов списка и правильная запись в эти узлы
	for (int i = 1; i < PL_SIZE; ++i) //Итератор i - узел в списке
	{
		current->next = (Node*)malloc(sizeof(Node));
		current->next->prev = current;
		current = current->next;
		current->data = NULL;
		for (int j = 0; j < PL_SIZE; ++j)
		{
			bool flag = 0; //не находится ли уже рассматриваемый элемент массива в списке. 1 - не находится
			for (int k = 0; k < 15; ++k) //сравниваем имена
			{
				if (pl[j].name[k] > current->prev->data->name[k])
				{
					flag = 1;
					break;
				}
				else if (pl[j].name[k] < current->prev->data->name[k])
					break;
			}
			if (flag && current->data == NULL)
				current->data = (PL*)&pl[j];
			else if (flag)
				for (int k = 0; k < 15; ++k)
				{
					if (pl[j].name[k] < current->data->name[k])
					{
						current->data = (PL*)&pl[j];
						break;
					}
					else if (pl[j].name[k] > current->data->name[k])
						break;
				}
		}
	}
	current->next = NULL;
	printPLlist(listPL);
	current = current->prev;
	for (; current != NULL; current = current->prev)
		delete current->next;
	delete listPL;
}

void printSalaryOrder(const PL *pl) //вывод в порядке убывания ЗП. В этой функции создается список по возрастанию ЗП, потом с конца выводится этот список
{
	//список будет по возрастанию, но выводить буду от последнего к первому элементу списка
	Node *listPL = (Node*)malloc(sizeof(Node)); //указатель на начало списка
	listPL->data = NULL;
	listPL->prev = NULL; //тк первый элемент списка
	Node *current = listPL;
	current->data = (PL*)pl; //"pl" - указатель на массив (т.е. указатель на первый элемент массива)
	for (int j = 1; j < PL_SIZE; ++j) //сравниваем имена. Итератор j - номер элемент в массиве
	{
		if (pl[j].avgSalary < current->data->avgSalary) //сравниваем средние ЗП двух ЯП
			current->data = (PL*)&pl[j];
	}
	//далее идёт алгоритм выделения памяти для узлов списка и правильная запись в эти узлы
	for (int i = 1; i < PL_SIZE; ++i) //Итератор i - узел в списке
	{
		current->next = (Node*)malloc(sizeof(Node));
		current->next->prev = current;
		current = current->next;
		current->data = NULL;
		for (int j = 0; j < PL_SIZE; ++j)
		{
			bool flag = 1; //не находится ли уже рассматриваемый элемент массива в списке. 1 - не находится
			if (pl[j].avgSalary < current->prev->data->avgSalary)
				continue;
			else if (pl[j].avgSalary == current->prev->data->avgSalary) //здесь может встретиться элемент, который уже был в списке, а может встретиться элемент, у которого такая же ЗП
			{                                                           //Поэтому пройду по списку пока не дойду до более низких ЗП или начала списка
				bool flag; //если flag == 1, то у нас разные элементы
				Node *locCurrent = current; //локальный указатель на узел
				while (locCurrent->prev->data->avgSalary == pl[j].avgSalary) //пока ЗП узла и рассмотриваемого элемента массива равны
				{
					if (&pl[j] == locCurrent->prev->data) //один и тот же элемент ли? сравниваем их адреса
					{
						flag = 0;
						break;
					}
					else
						flag = 1;
					if (locCurrent->prev->prev == NULL)
						break;
					locCurrent = locCurrent->prev;
				}
				if (flag)
				{
					current->data = (PL*)&pl[j]; //судя по всему из константы в неконстанту неявно типы не приводятся, поэтому приводил тип явно
					break;
				}
			}
			else
			{
				if (!current->data) //если узел пуст - присваиваем
					current->data = (PL*)&pl[j];
				else if (pl[j].avgSalary < current->data->avgSalary)
					current->data = (PL*)&pl[j];
			}
		}
	}
	current->next = NULL;
	printRevPLlist(listPL);
	current = current->prev;
	for (; current != NULL; current = current->prev)
		delete current->next;
	delete listPL;
}

void printPopularityOrder(const PL *pl)
{
	//список будет по возрастанию, но выводить буду от последнего к первому элементу списка
	Node *listPL = (Node*)malloc(sizeof(Node)); //указатель на начало списка
	listPL->data = NULL;
	listPL->prev = NULL; //тк первый элемент списка
	Node *current = listPL;
	current->data = (PL*)pl; //"pl" - указатель на массив (т.е. указатель на первый элемент массива)
	for (int j = 1; j < PL_SIZE; ++j) //Итератор j - номер элемент в массиве
	{
		if (pl[j].popularity < current->data->popularity) //сравниваем популярности двух ЯП
			current->data = (PL*)&pl[j];
	}
	//далее идёт алгоритм выделения памяти для узлов списка и правильная запись в эти узлы
	for (int i = 1; i < PL_SIZE; ++i) //Итератор i - узел в списке
	{
		current->next = (Node*)malloc(sizeof(Node));
		current->next->prev = current;
		current = current->next;
		current->data = NULL;
		for (int j = 0; j < PL_SIZE; ++j)
		{
			bool flag = 1; //не находится ли уже рассматриваемый элемент массива в списке. 1 - не находится
			if (pl[j].popularity < current->prev->data->popularity)
				continue;
			else if (pl[j].popularity == current->prev->data->popularity) //здесь может встретиться элемент, который уже был в списке, а может встретиться элемент, у которого такая же популярность
			{                                                             //Поэтому пройду по списку пока не дойду до более низких ЗП или начала списка
				bool flag; //если flag == 1, то у нас разные элементы
				Node *locCurrent = current; //локальный указатель на узел
				while (locCurrent->prev->data->popularity == pl[j].popularity) //пока популярность узла и рассмотриваемого элемента массива равны
				{
					if (&pl[j] == locCurrent->prev->data) //один и тот же элемент ли? сравниваем их адреса
					{
						flag = 0;
						break;
					}
					else
						flag = 1;
					if (locCurrent->prev->prev == NULL)
						break;
					locCurrent = locCurrent->prev;
				}
				if (flag)
				{
					current->data = (PL*)&pl[j]; //из константы в неконстанту неявно типы не приводятся, так как потом по этому неконстантному указателю можем случайно изменить данные, поэтому приводил тип явно
					break;
				}
			}
			else
			{
				if (!current->data) //если узел пуст - присваиваем
					current->data = (PL*)&pl[j];
				else if (pl[j].popularity < current->data->popularity)
					current->data = (PL*)&pl[j];
			}
		}
	}
	current->next = NULL;
	printRevPLlist(listPL);
	current = current->prev;
	for (; current != NULL; current = current->prev)
		delete current->next;
	delete listPL;
}

void printCompiledFirst(const PL *pl)
{
	Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
	Console::Clear();
	Console::ForegroundColor = ConsoleColor::White; //выбираем цвет текста
	printf("               |                  |                  |Популярность|Средняя|Появился|  Web  |Mobile | Game  |Desktop\n");
	printf("      ЯП       |    Транслятор    |    Типизация     |  (0-100)   |ЗП(руб)| (Год)  |Develop|Develop|Develop|Develop\n");
	printf("===============+==================+==================+============+=======+========+=======+=======+=======+=======\n");
	for (int i = 0; i < PL_SIZE; ++i)
	{
		if (pl[i].isCompiled)
		{
			printf("%-15s|  Компилятор      |   %-15s|    %-8hu|", pl[i].name, pl[i].isStatic?"Статическая":"Динамическая", pl[i].popularity);
			printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
			printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[i].area.webDev?'+':'-', pl[i].area.mobileDev?'+':'-', pl[i].area.gameDev?'+':'-', pl[i].area.desktopDev?'+':'-');
		}
	}
	for (int i = 0; i < PL_SIZE; ++i)
	{
		if (!pl[i].isCompiled)
		{
			printf("%-15s|  Интерпретатор   |   %-15s|    %-8hu|", pl[i].name, pl[i].isStatic?"Статическая":"Динамическая", pl[i].popularity);
			printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
			printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[i].area.webDev?'+':'-', pl[i].area.mobileDev?'+':'-', pl[i].area.gameDev?'+':'-', pl[i].area.desktopDev?'+':'-');
		}
	}
}

void printStaticFirst(const PL *pl)
{
	Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
	Console::Clear();
	Console::ForegroundColor = ConsoleColor::White; //выбираем цвет текста
	printf("               |                  |                  |Популярность|Средняя|Появился|  Web  |Mobile | Game  |Desktop\n");
	printf("      ЯП       |    Транслятор    |    Типизация     |  (0-100)   |ЗП(руб)| (Год)  |Develop|Develop|Develop|Develop\n");
	printf("===============+==================+==================+============+=======+========+=======+=======+=======+=======\n");
	for (int i = 0; i < PL_SIZE; ++i)
		if (pl[i].isStatic)
		{		
			printf("%-15s|  %-16s|   Статическая    |    %-8hu|", pl[i].name, pl[i].isCompiled?"Компилятор":"Интерпретатор", pl[i].popularity);
			printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
			printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[i].area.webDev?'+':'-', pl[i].area.mobileDev?'+':'-', pl[i].area.gameDev?'+':'-', pl[i].area.desktopDev?'+':'-');
		}
	for (int i = 0; i < PL_SIZE; ++i)
		if (!pl[i].isStatic)
		{
			printf("%-15s|  %-16s|   Динамическая   |    %-8hu|", pl[i].name, pl[i].isCompiled?"Компилятор":"Интерпретатор", pl[i].popularity);
			printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
			printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[i].area.webDev?'+':'-', pl[i].area.mobileDev?'+':'-', pl[i].area.gameDev?'+':'-', pl[i].area.desktopDev?'+':'-');
		}
}

void printOldFirst(const PL *pl)
{
	Node *listPL = (Node*)malloc(sizeof(Node)); //указатель на начало списка
	listPL->data = NULL;
	listPL->prev = NULL; //тк первый элемент списка
	Node *current = listPL;
	current->data = (PL*)pl; //"pl" - указатель на массив (т.е. указатель на первый элемент массива)
	for (int j = 1; j < PL_SIZE; ++j) //Итератор j - номер элемент в массиве
	{
		if (pl[j].appeared < current->data->appeared) //сравниваем годы основания двух ЯП
			current->data = (PL*)&pl[j];
	}
	//далее идёт алгоритм выделения памяти для узлов списка и правильная запись в эти узлы
	for (int i = 1; i < PL_SIZE; ++i) //Итератор i - узел в списке
	{
		current->next = (Node*)malloc(sizeof(Node));
		current->next->prev = current;
		current = current->next;
		current->data = NULL;
		for (int j = 0; j < PL_SIZE; ++j)
		{
			bool flag = 1; //не находится ли уже рассматриваемый элемент массива в списке. 1 - не находится
			if (pl[j].appeared < current->prev->data->appeared)
				continue;
			else if (pl[j].appeared == current->prev->data->appeared) //здесь может встретиться элемент, который уже был в списке, а может встретиться элемент, у которого тот же год основания
			{                                                         //Поэтому пройду по списку пока не дойду до более старого ЯП или начала списка
				bool flag; //если flag == 1, то у нас разные элементы
				Node *locCurrent = current; //локальный указатель на узел
				while (locCurrent->prev->data->appeared == pl[j].appeared) //пока годы основания узла и рассмотриваемого элемента массива равны
				{
					if (&pl[j] == locCurrent->prev->data) //один и тот же элемент ли? сравниваем их адреса
					{
						flag = 0;
						break;
					}
					else
						flag = 1;
					if (locCurrent->prev->prev == NULL)
						break;
					locCurrent = locCurrent->prev;
				}
				if (flag)
				{
					current->data = (PL*)&pl[j]; //судя по всему из константы в неконстанту неявно типы не приводятся, поэтому приводил тип явно
					break;
				}
			}
			else
			{
				if (!current->data) //если узел пуст - присваиваем
					current->data = (PL*)&pl[j];
				else if (pl[j].appeared < current->data->appeared)
					current->data = (PL*)&pl[j];
			}
		}
	}
	current->next = NULL;
	printPLlist(listPL);
	current = current->prev;
	for (; current != NULL; current = current->prev)
		delete current->next;
	delete listPL;
}

void printPLappArea(const PL *pl, int appArea) //выводит только языки для конкретной области разработки (appArea: 1 - веб, 2 - мобильная, 3 - игры, 4 - десктоп)
{
	Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
	Console::Clear();
	Console::ForegroundColor = ConsoleColor::White; //выбираем цвет текста
	printf("               |                  |                  |Популярность|Средняя|Появился|  Web  |Mobile | Game  |Desktop\n");
	printf("      ЯП       |    Транслятор    |    Типизация     |  (0-100)   |ЗП(руб)| (Год)  |Develop|Develop|Develop|Develop\n");
	printf("===============+==================+==================+============+=======+========+=======+=======+=======+=======\n");
	if (appArea == 1)
	{
		for (int i = 0; i < PL_SIZE; ++i)
			if (pl[i].area.webDev)
			{
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[i].name, pl[i].isCompiled?"Компилятор":"Интерпретатор", pl[i].isStatic?"Статическая":"Динамическая", pl[i].popularity);
				printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
				printf("   +   |   %-4c|   %-4c|   %-4c\n", pl[i].area.mobileDev?'+':'-', pl[i].area.gameDev?'+':'-', pl[i].area.desktopDev?'+':'-');
			}
	}
	else if (appArea == 2)
	{
		for (int i = 0; i < PL_SIZE; ++i)
			if (pl[i].area.mobileDev)
			{
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[i].name, pl[i].isCompiled?"Компилятор":"Интерпретатор", pl[i].isStatic?"Статическая":"Динамическая", pl[i].popularity);
				printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
				printf("   %-4c|   +   |   %-4c|   %-4c\n", pl[i].area.webDev?'+':'-', pl[i].area.gameDev?'+':'-', pl[i].area.desktopDev?'+':'-');
			}
	}
	else if (appArea == 3)
	{
		for (int i = 0; i < PL_SIZE; ++i)
			if (pl[i].area.gameDev)
			{
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[i].name, pl[i].isCompiled?"Компилятор":"Интерпретатор", pl[i].isStatic?"Статическая":"Динамическая", pl[i].popularity);
				printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
				printf("   %-4c|   %-4c|   +   |   %-4c\n", pl[i].area.webDev?'+':'-', pl[i].area.mobileDev?'+':'-', pl[i].area.desktopDev?'+':'-');
			}
	}
	else if (appArea == 4)
	{
		for (int i = 0; i < PL_SIZE; ++i)
			if (pl[i].area.desktopDev)
			{
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[i].name, pl[i].isCompiled?"Компилятор":"Интерпретатор", pl[i].isStatic?"Статическая":"Динамическая", pl[i].popularity);
				printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
				printf("   %-4c|   %-4c|   %-4c|   +   \n", pl[i].area.webDev?'+':'-', pl[i].area.mobileDev?'+':'-', pl[i].area.gameDev?'+':'-');
			}
	}
}

bool addPL(PL *p2pl)
{
	PL pl; //на следующей строке настрою default
	pl.appeared = 0; pl.area.desktopDev = 0; pl.area.gameDev = 0; pl.area.mobileDev = 0; pl.area.webDev = 0; pl.avgSalary = 0; pl.isCompiled = 1; pl.isStatic = 1; pl.name[0] = '\0'; pl.popularity = 0;
	int nameI = 0, popI = 0, salI = 0, appeI = 0; //итераторы. static не подойдёт, так как при повторном вызове функции всё полетит
	const unsigned short M_SIZE = 11; //кол-во строк в меню добавления ЯП
	char addPLmenu[M_SIZE][80] =
	{
		"     Название языка программирования (eng, символов < 15):                     ",
		"     Язык компилируемый или интерпретируемый?                                  ",
		"     Язык статически или динамически типизированный?                           ",
		"     Баллы популярности языка (0 < баллы < 101):                               ",
		"     Средняя ЗП для данного языка (в тыс)(49k < ЗП < 251k):                    ",
		"     Год появления языка (1940 < Год появления < 2022):                        ",
		"     Язык применяется в \"Веб разработка\"?                                      ",
		"     Язык применяется в \"Мобильная разработка\"?                                ",
		"     Язык применяется в \"Разработка игр\"?                                      ",
		"     Язык применяется в \"Разработка настольных приложений\"?                    ",
		"                    Добавить язык программирования в файл.                     "
	};
	Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
	Console::Clear();
	Console::CursorLeft = 10;
	Console::CursorTop = 4;
	Console::ForegroundColor = fNIGround; //выбираем цвет текста для неинтерактивного поля
	Console::BackgroundColor = bNIGround; //выбираем цвет фона для неинтерактивного поля
	printf("                       ДОБАВЛЕНИЯ ЯЗЫКА ПРОГРАММИРОВАНИЯ:                      ");
	Console::ForegroundColor = fGround; //выбираем цвет текста
	Console::BackgroundColor = bGround; //выбираем цвет фона
	for (int i = 0; i < M_SIZE; ++i) //выводим главное меню
		{
			Console::CursorLeft = 10;
			Console::CursorTop = 5 + i;
			printf("%s", addPLmenu[i]);
		}
	Console::CursorLeft = 71; Console::CursorTop = 6; printf("%s", pl.isCompiled?"Компилируемый   ":"Интерпретируемый"); //вывод default
	Console::CursorLeft = 71; Console::CursorTop = 7; printf("%s", pl.isStatic?"Статически ":"Динамически");
	Console::CursorLeft = 71; Console::CursorTop = 11; printf("%s", pl.area.webDev?"да ":"нет");
	Console::CursorLeft = 71; Console::CursorTop = 12; printf("%s", pl.area.mobileDev?"да ":"нет");
	Console::CursorLeft = 71; Console::CursorTop = 13; printf("%s", pl.area.gameDev?"да ":"нет");
	Console::CursorLeft = 71; Console::CursorTop = 14; printf("%s", pl.area.desktopDev?"да ":"нет"); //конец вывода default
	Console::CursorLeft = 10;
	Console::CursorTop = 5 + M_SIZE;
	unsigned short y = 0; //текущий пункт меню
	unsigned short prevY = 0; //предыдущий пункт меню
	Console::ForegroundColor = fGround;
	Console::BackgroundColor = bCGround; //цвет фона текущей строки (на которой сейчас стоит "курсор")
	Console::CursorLeft = 10;
	Console::CursorTop = y + 5;
	printf("%s", addPLmenu[y]);
	while (true)
	{
		switch (getch())
		{
		case CURSOR_KEYS: //чтобы работали только стрелки (без 'P', 'H' и функциональных клавиш)
			prevY = y;
			switch (getch())
			{
			case DOWN:
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", addPLmenu[y]);
				y == M_SIZE - 1 ? y = 0 : y++;
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bCGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", addPLmenu[y]);
				break;
			case UP:
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", addPLmenu[y]);
				y == 0 ? y = M_SIZE - 1 : y--;
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bCGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", addPLmenu[y]);
				break;
			case HOME:
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", addPLmenu[y]);
				y = 0;
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bCGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", addPLmenu[y]);
				break;
			case END:
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", addPLmenu[y]);
				y = M_SIZE - 1;
				Console::ForegroundColor = fGround;
				Console::BackgroundColor = bCGround;
				Console::CursorLeft = 10;
				Console::CursorTop = y + 5;
				printf("%s", addPLmenu[y]);
				break;
			}
			switch (y)
			{
			case 0: Console::CursorLeft = 71; Console::CursorTop = 5; Console::CursorLeft = 71; Console::CursorTop = 5; printf("%s", pl.name); break;
			case 1: Console::CursorLeft = 71; Console::CursorTop = 6; printf("%s", pl.isCompiled?"Компилируемый   ":"Интерпретируемый"); break;
			case 2: Console::CursorLeft = 71; Console::CursorTop = 7; printf("%s", pl.isStatic?"Статически ":"Динамически"); break;
			case 3: Console::CursorLeft = 71; Console::CursorTop = 8; if (pl.popularity!= 0) printf("%hu", pl.popularity); break;
			case 4: Console::CursorLeft = 71; Console::CursorTop = 9; if (pl.avgSalary!= 0) printf("%u", pl.avgSalary); break;
			case 5: Console::CursorLeft = 71; Console::CursorTop = 10; if (pl.appeared!= 0) printf("%hu", pl.appeared); break;
			case 6: Console::CursorLeft = 71; Console::CursorTop = 11; printf("%s", pl.area.webDev?"да ":"нет"); break;
			case 7: Console::CursorLeft = 71; Console::CursorTop = 12; printf("%s", pl.area.mobileDev?"да ":"нет"); break;
			case 8: Console::CursorLeft = 71; Console::CursorTop = 13; printf("%s", pl.area.gameDev?"да ":"нет"); break;
			case 9: Console::CursorLeft = 71; Console::CursorTop = 14; printf("%s", pl.area.desktopDev?"да ":"нет"); break;
			}
			Console::BackgroundColor = bGround;
			switch (prevY)
			{
			case 0: Console::CursorLeft = 71; Console::CursorTop = 5; Console::CursorLeft = 71; Console::CursorTop = 5; printf("%s", pl.name); break;
			case 1: Console::CursorLeft = 71; Console::CursorTop = 6; printf("%s", pl.isCompiled?"Компилируемый   ":"Интерпретируемый"); break;
			case 2: Console::CursorLeft = 71; Console::CursorTop = 7; printf("%s", pl.isStatic?"Статически ":"Динамически"); break;
			case 3: Console::CursorLeft = 71; Console::CursorTop = 8; if (pl.popularity!= 0) printf("%hu", pl.popularity); break;
			case 4: Console::CursorLeft = 71; Console::CursorTop = 9; if (pl.avgSalary!= 0) printf("%u", pl.avgSalary); break;
			case 5: Console::CursorLeft = 71; Console::CursorTop = 10; if (pl.appeared!= 0) printf("%hu", pl.appeared); break;
			case 6: Console::CursorLeft = 71; Console::CursorTop = 11; printf("%s", pl.area.webDev?"да ":"нет"); break;
			case 7: Console::CursorLeft = 71; Console::CursorTop = 12; printf("%s", pl.area.mobileDev?"да ":"нет"); break;
			case 8: Console::CursorLeft = 71; Console::CursorTop = 13; printf("%s", pl.area.gameDev?"да ":"нет"); break;
			case 9: Console::CursorLeft = 71; Console::CursorTop = 14; printf("%s", pl.area.desktopDev?"да ":"нет"); break;
			}
			Console::BackgroundColor = bCGround;
			break;
		case ENTER:
			switch (y)
			{
			case 0: Console::CursorTop = 5; Console::CursorVisible::set(true);
				while (true)
				{
					Console::CursorLeft = 71 + nameI;
					char ch = getch();
					Console::CursorVisible::set(false); Console::CursorLeft = 90;
					Console::BackgroundColor = ConsoleColor::Black; printf("                    "); Console::BackgroundColor = bCGround; //стираем надпись о том, что ЯП уже добавлен
					Console::CursorVisible::set(true);
					Console::CursorLeft = 71 + nameI;
					if (ch == ENTER && nameI != 0) { pl.name[nameI] = '\0'; if (!comparePL(p2pl, pl.name)) break; else { Console::CursorLeft = 90; printf("Этот ЯП уже добавлен"); } }
					else if (ch == BACKSPACE && nameI > 0) { --nameI; Console::CursorLeft = 71 + nameI; printf(" "); Console::CursorLeft = 71 + nameI; }
					if (nameI == 14) continue;
					else if (ch == 0 || ch == (char)CURSOR_KEYS) getch(); //избегаем ошибок, так как getch() в случае нажатия, к примеру, f1 положит в поток 0 и 59, а 59 в ascii - ';'
					else if (ch < 127 && ch > 32) { pl.name[nameI] = ch; printf("%c", ch); ++nameI; }
				}
				Console::CursorVisible::set(false); break;
			case 1: pl.isCompiled = !pl.isCompiled; Console::CursorLeft = 71; Console::CursorTop = 6; printf("%s", pl.isCompiled?"Компилируемый   ":"Интерпретируемый"); break;
			case 2: pl.isStatic = !pl.isStatic; Console::CursorLeft = 71; Console::CursorTop = 7; printf("%s", pl.isStatic?"Статически ":"Динамически"); break;
			case 3: Console::CursorTop = 8; Console::CursorVisible::set(true);
				while (true)
				{
					char pop[4];
					Console::CursorLeft = 71 + popI;
					char ch = getch();
					if (ch == ENTER) { pop[popI] = '\0'; if (atoi(pop) > 0 && atoi(pop) < 101) { pl.popularity = atoi(pop); break; } }
					else if (ch == BACKSPACE && popI > 0) { --popI; Console::CursorLeft = 71 + popI; printf(" "); Console::CursorLeft = 71 + popI; }
					if (popI == 3) continue;
					else if (ch == 0 || ch == (char)CURSOR_KEYS) getch(); //избегаем ошибок, так как getch() в случае нажатия, к примеру, f1 положит в поток 0 и 59, а 59 в ascii - ';'
					else if (ch < 58 && ch > 47) { pop[popI] = ch; printf("%c", ch); ++popI; }
				}
				Console::CursorVisible::set(false); break;
			case 4: Console::CursorTop = 9; Console::CursorVisible::set(true);
				while (true)
				{
					char sal[4];
					Console::CursorLeft = 71 + salI;
					char ch = getch();
					if (ch == ENTER) { sal[salI] = '\0'; if (atoi(sal) > 49 && atoi(sal) < 251) { pl.avgSalary = atoi(sal) * 1000; break; } }
					else if (ch == BACKSPACE && salI > 0) { --salI; Console::CursorLeft = 71 + salI; printf(" "); Console::CursorLeft = 71 + salI; }
					if (salI == 3) continue;
					else if (ch == 0 || ch == (char)CURSOR_KEYS) getch(); //избегаем ошибок, так как getch() в случае нажатия, к примеру, f1 положит в поток 0 и 59, а 59 в ascii - ';'
					else if (ch < 58 && ch > 47) { sal[salI] = ch; printf("%c", ch); ++salI; }
				}
				Console::CursorVisible::set(false); break;
			case 5: Console::CursorTop = 10; Console::CursorVisible::set(true);
				while (true)
				{
					char appe[5];
					Console::CursorLeft = 71 + appeI;
					char ch = getch();
					if (ch == ENTER) { appe[appeI] = '\0'; if (atoi(appe) > 1940 && atoi(appe) < 2022) { pl.appeared = atoi(appe); break; } }
					else if (ch == BACKSPACE && appeI > 0) { --appeI; Console::CursorLeft = 71 + appeI; printf(" "); Console::CursorLeft = 71 + appeI; }
					if (appeI == 4) continue;
					else if (ch == 0 || ch == (char)CURSOR_KEYS) getch(); //избегаем ошибок, так как getch() в случае нажатия, к примеру, f1 положит в поток 0 и 59, а 59 в ascii - ';'
					else if (ch < 58 && ch > 47) { appe[appeI] = ch; printf("%c", ch); ++appeI; }
				}
				Console::CursorVisible::set(false); break;
			case 6: pl.area.webDev = !pl.area.webDev; Console::CursorLeft = 71; Console::CursorTop = 11; printf("%s", pl.area.webDev?"да ":"нет"); break;
			case 7: pl.area.mobileDev = !pl.area.mobileDev; Console::CursorLeft = 71; Console::CursorTop = 12; printf("%s", pl.area.mobileDev?"да ":"нет"); break;
			case 8: pl.area.gameDev = !pl.area.gameDev; Console::CursorLeft = 71; Console::CursorTop = 13; printf("%s", pl.area.gameDev?"да ":"нет"); break;
			case 9: pl.area.desktopDev = !pl.area.desktopDev; Console::CursorLeft = 71; Console::CursorTop = 14; printf("%s", pl.area.desktopDev?"да ":"нет"); break;
			case 10: Console::CursorLeft = 30; Console::CursorTop = 15; printf("                                      ");
				Console::CursorLeft = 33; Console::CursorTop = 15; printf("Нажмите ENTER для подтверждения.");
				if (getch() == ENTER && pl.appeared != 0 && pl.avgSalary != 0 && pl.name[0] != '\0' && pl.popularity != 0)
				{ save2File(&pl); return true; } else { Console::CursorLeft = 30; printf("Добавить язык программирования в файл."); }
			}
			break;
		case ESC:
			return false; //выход из меню добавления ЯП
		}
	}
}

bool comparePL(const PL *pl, const char name[15]) //сравнивает предполагаемое имя нового ЯП с остальными. Вернёт 1, если ЯП с таким именем уже есть
{
	for (int i = 0; i < PL_SIZE; ++i)
		if (!strcmp(pl[i].name, name))
			return true;
	return false;
}

bool remPL(PL *pl)
{
	Console::BackgroundColor = ConsoleColor::Black; //выбираем цвет фона для очистки консоли
	Console::Clear();
	Console::ForegroundColor = ConsoleColor::White; //выбираем цвет текста
	printf("               |                  |                  |Популярность|Средняя|Появился|  Web  |Mobile | Game  |Desktop\n");
	printf("      ЯП       |    Транслятор    |    Типизация     |  (0-100)   |ЗП(руб)| (Год)  |Develop|Develop|Develop|Develop\n");
	printf("===============+==================+==================+============+=======+========+=======+=======+=======+=======\n");
	for (int i = 0; i < PL_SIZE; ++i)
	{
		printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[i].name, pl[i].isCompiled?"Компилятор":"Интерпретатор", pl[i].isStatic?"Статическая":"Динамическая", pl[i].popularity);
		printf("%-7u|  %-6hu|", pl[i].avgSalary, pl[i].appeared);
		printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[i].area.webDev?'+':'-', pl[i].area.mobileDev?'+':'-', pl[i].area.gameDev?'+':'-', pl[i].area.desktopDev?'+':'-');
	}
	Console::CursorLeft = 0; Console::CursorTop = 3;
	int y = 0;
	Console::ForegroundColor = ConsoleColor::Black;
	Console::BackgroundColor = ConsoleColor::White;
	Console::CursorLeft = 0;
	Console::CursorTop = y + 3;
	printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
	printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
	printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
	Console::ForegroundColor = ConsoleColor::Black; Console::BackgroundColor = ConsoleColor::White;
	Console::CursorLeft = 40; Console::CursorTop = PL_SIZE + 4; printf("Выберите удаляемый язык программирования.");
	while (true)
	{
		switch(getch())
		{
		case CURSOR_KEYS:
			switch (getch())
			{
			case DOWN:
				Console::BackgroundColor = ConsoleColor::Black;
				Console::ForegroundColor = ConsoleColor::White;
				Console::CursorLeft = 0;
				Console::CursorTop = y + 3;
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
				printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
				printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
				y == PL_SIZE - 1 ? y = 0 : y++;
				Console::ForegroundColor = ConsoleColor::Black;
				Console::BackgroundColor = ConsoleColor::White;
				Console::CursorLeft = 0;
				Console::CursorTop = y + 3;
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
				printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
				printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
				break;
			case UP:
				Console::BackgroundColor = ConsoleColor::Black;
				Console::ForegroundColor = ConsoleColor::White;
				Console::CursorLeft = 0;
				Console::CursorTop = y + 3;
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
				printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
				printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
				y == 0 ? y = PL_SIZE - 1 : y--;
				Console::ForegroundColor = ConsoleColor::Black;
				Console::BackgroundColor = ConsoleColor::White;
				Console::CursorLeft = 0;
				Console::CursorTop = y + 3;
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
				printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
				printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
				break;
			case HOME:
				Console::BackgroundColor = ConsoleColor::Black;
				Console::ForegroundColor = ConsoleColor::White;
				Console::CursorLeft = 0;
				Console::CursorTop = y + 3;
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
				printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
				printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
				y = 0;
				Console::ForegroundColor = ConsoleColor::Black;
				Console::BackgroundColor = ConsoleColor::White;
				Console::CursorLeft = 0;
				Console::CursorTop = y + 3;
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
				printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
				printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
				break;
			case END:
				Console::BackgroundColor = ConsoleColor::Black;
				Console::ForegroundColor = ConsoleColor::White;
				Console::CursorLeft = 0;
				Console::CursorTop = y + 3;
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
				printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
				printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
				y = PL_SIZE - 1;
				Console::ForegroundColor = ConsoleColor::Black;
				Console::BackgroundColor = ConsoleColor::White;
				Console::CursorLeft = 0;
				Console::CursorTop = y + 3;
				printf("%-15s|  %-16s|   %-15s|    %-8hu|", pl[y].name, pl[y].isCompiled?"Компилятор":"Интерпретатор", pl[y].isStatic?"Статическая":"Динамическая", pl[y].popularity);
				printf("%-7u|  %-6hu|", pl[y].avgSalary, pl[y].appeared);
				printf("   %-4c|   %-4c|   %-4c|   %-4c\n", pl[y].area.webDev?'+':'-', pl[y].area.mobileDev?'+':'-', pl[y].area.gameDev?'+':'-', pl[y].area.desktopDev?'+':'-');
				break;
			}
			break;
		case ENTER:
			Console::ForegroundColor = ConsoleColor::Black;
			Console::BackgroundColor = ConsoleColor::White;
			Console::CursorLeft = 40;
			Console::CursorTop = PL_SIZE + 4;
			printf("Нажмите ENTER для подтверждения удаления.");
			if (getch() == ENTER) { delFromFile(y, pl); return true; }
			else { Console::CursorLeft = 40; Console::CursorTop = PL_SIZE + 4; printf("Выберите удаляемый язык программирования."); }
			break;
		case ESC:
			return false;
		}
	}
}