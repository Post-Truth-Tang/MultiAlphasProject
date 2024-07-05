#include "LRPred.h"
#include <fstream>

#define MAXLEN 1024000

#include <cstdio>
#include <unistd.h>
#include <string>

int main(int argc, char *argv[]) {
//    const char * namesPath = "C:/Users/John/source/repos/CMakeProject1/CMakeProject1/names2.txt";
//    const char * data2Path = "C:/Users/John/source/repos/CMakeProject1/CMakeProject1/data2.csv";
    const char * namesPath = "C:/Users/tqz_trader/Desktop/AlphaProject/names.txt";
    const char * data2Path = "C:/Users/tqz_trader/Desktop/AlphaProject/data2.csv";

    auto* lrPred = new LRPred(namesPath);
    lrPred->InitPred();
    lrPred->SetCoreCounts(2);

	//读文件
	long length = 0;
	auto *data = new Snapshot[MAXLEN];
	Snapshot *curr = data;

    ifstream myfile(data2Path);
	if (!myfile) {
		cout << "Unable to open myfile";
		exit(1); // terminate with error
	}

    string line2;
	//getline(myfile, line2);
	while (!myfile.eof() && length < MAXLEN) {
		getline(myfile, line2);  //读入每行
		sscanf(line2.c_str(),
			"%d,%d,%ld,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%lf,%lf\n",
			&curr->dsrc, &curr->security, &curr->timestamp,
			&curr->level, &curr->last,
			&curr->bid, &curr->bid1, &curr->bid2, &curr->bid3, &curr->bid4,
			&curr->ask, &curr->ask1, &curr->ask2, &curr->ask3, &curr->ask4,
			&curr->bz, &curr->bz1, &curr->bz2, &curr->bz3, &curr->bz4,
			&curr->az, &curr->az1, &curr->az2, &curr->az3, &curr->az4,
			&curr->volume, &curr->acc_volume,
			&curr->turnover, &curr->acc_turnover);

		++curr;
		++length;
	}

	for (int i = 1; i < 10; i++) {
        lrPred->UpdateTick(data[i]);

        lrPred->UpdateAlphas(); // 内置线程

		lrPred->Update(data[i]);
		lrPred->UpdatePrevData();
	}

    delete lrPred;
    return 0;
}
