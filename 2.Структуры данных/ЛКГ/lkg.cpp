#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#define a 6364136223846793005
#define c 11

unsigned long long lkg(unsigned long long x) {
	unsigned long long xo = (x * a + c);
	return xo;
}
/*unsigned long long period1() {
	srand(time(NULL));
	unsigned long long x0 = rand();
	unsigned long long x1 = lkg(x0);
	unsigned long long x = lkg(x1);
	unsigned long long period = 1;
	while(x != x1){
		x = lkg(x);
		period++;
	}
	return period;
}*/
float poker(int r, int s, int d) {
	float t = 1;
	for (int i = 0; i <= r - 1; i++)
		t *= (d - i);
	float prob = t / pow(d, 5) * s;
	return prob;
}
int power() {
	int powerr = 1;
	unsigned long long a1 = a - 1;
	unsigned long long p = a1;
	while (a1 != 0) {
		a1 = a1 * p;
		powerr++;
	}
	return powerr;
}

void hi() {
	int p1 = 0, p2 = 0, p3 = 0, p4 = 0;
	srand(time(NULL));
	unsigned long long x0 = rand();
	unsigned long long x = lkg(x0);
	// float np = 8000. / 8;
	int np = 1000;
	for (int j = 0; j < 1000; j++) {
		unsigned int arr[8] = { 0 };
		for (int i = 0; i < 8000; i++) {
			x = lkg(x);
			unsigned long long max = 18446740473709551615;
			if (0 <= x && x < max / 8)arr[0]++;
			if (max / 8 <= x && x < max / 4)arr[1]++;
			if (max / 4 <= x && x < (max / 8) * 3)arr[2]++;
			if ((max / 8) * 3 <= x && x < max / 2)arr[3]++;
			if (max / 2 <= x && x < (max / 8) * 5)arr[4]++;
			if ((max / 8) * 5 <= x && x < (max / 4) * 3)arr[5]++;
			if ((max / 4) * 3 <= x && x < (max / 8) * 7)arr[6]++;
			if ((max / 8) * 7 <= x && x < max)arr[7]++;
		}
		x = lkg(x);
		double hi_kvadrat = 0;
		for (int i = 0; i < 8; i++)
			hi_kvadrat = hi_kvadrat + (double)(((arr[i] - np) * (arr[i] - np)) / (double)np);
		// printf("%f\n", hi_kvadrat);
		if (1.239 >= hi_kvadrat || hi_kvadrat >= 18.48)p1++;
		if (1.239 < hi_kvadrat && hi_kvadrat <= 2.167 || 14.07 <= hi_kvadrat && hi_kvadrat < 18.48)p2++;
		if (2.167 < hi_kvadrat && hi_kvadrat <= 2.833 || 12.017 <= hi_kvadrat && hi_kvadrat < 14.07)p3++;
		if (2.833 < hi_kvadrat && hi_kvadrat < 12.017)p4++;
	}

	printf("Хи_квадрат \"очень хороший\": %d\nХи_квадрат \"хороший\": %d\nХи_квадрат \"подозрительный\": %d\nХи_квадрат \"очень плохой\": %d\n", p4, p3, p2, p1);
}

void poker_poker() {
	int pp1 = 0, pp2 = 0, pp3 = 0, pp4 = 0;
	float aa[5] = { 0 };
	aa[0] = poker(1, 1, 11) * 10000;
	aa[1] = poker(2, 15, 11) * 10000;
	aa[2] = poker(3, 25, 11) * 10000;
	aa[3] = poker(4, 10, 11) * 10000;
	aa[4] = poker(5, 1, 11) * 10000;
	srand(time(NULL));
	unsigned long long x0 = rand();
	unsigned long long x = lkg(x0);
	for (int i = 0; i < 1000; i++) {
		int arri[5] = { 0 };
		double hi_poker = 0;
		int kolvo[5] = { 0 };
		for (int i = 0; i < 10000; i++) {
			for (int j = 0; j < 5; j++) {
				x = lkg(x);
				arri[j] = x % (int)11;
			}
			for (int i = 1; i < 5; i++) {
				for (int j = 4; j >= i; j--) {
					if (arri[j - 1] > arri[j]) {
						int h = arri[j - 1];
						arri[j - 1] = arri[j];
						arri[j] = h;
					}
				}
			}
			int kol = 5;
			for (int i = 0; i < 5; i++) {
				for (int j = i + 1; j < 5; j++) {
					if (arri[i] == arri[j]) {
						kol--;
						i = j;
					}
				}
			}
			if (kol == 1) kolvo[0]++;//aaaaa
			if (kol == 2) kolvo[1]++;//aaaab
			if (kol == 3) kolvo[2]++;//aabbc
			if (kol == 4) kolvo[3]++;//abbcd
			if (kol == 5) kolvo[4]++;//abcde
		}

		for (int i = 0; i < 5; i++)
		{
			hi_poker = hi_poker + (double)(((kolvo[i] - aa[i]) * (kolvo[i] - aa[i])) / aa[i]);
		}
		if (0.297 >= hi_poker || hi_poker >= 13.277)pp1++;
		if (0.297 < hi_poker && hi_poker < 0.7107 || 9.488 <= hi_poker && hi_poker < 13.277)pp2++;
		if (0.7107 < hi_poker && hi_poker < 1.064 || 7.779 < hi_poker && hi_poker <= 9.488)pp3++;
		if (1.064 <= hi_poker && hi_poker <= 7.779)pp4++;

	}

	printf("Покер-критерий \"очень хороший\": %d\nПокер-критерий \"хороший\": %d\nПокер-критерий \"подозрительный\": %d\nПокер-критерий \"очень плохой\": %d\n", pp4, pp3, pp2, pp1);

}




int main() {
	clock_t start_time = clock();
	setlocale(LC_ALL, "Rus");
	//printf("Период^%llu\n", period1());
	printf("Мощность: %d\n", power());
	hi();
	poker_poker();
	clock_t end_time = clock();
	double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	printf("Время выполнения: %f секунд\n", elapsed_time);
	return 0;
}
