#include <bits/stdc++.h>
#include <cstdlib>
using namespace std;

int main(int argc, char *argv[]){
	if(argc!=2){
		cerr<<"Error: use \"./fileGenerator <total-files>\"\n";
		return 1;
	}
	int n=atoi(argv[1]);
	for(int i=0;i<=n;i++){
		string Result;
		stringstream convert;
		convert << i;
		Result = convert.str();
		string const file="files/foo"+Result+".txt";
		const char *file1 = file.c_str();
		ofstream out(file1,ofstream::out);
		for(int j=0;j<2000000;j++){
			out<<char(char(rand()%26)+'a');
		}
		cout<<"file"<<i<<" created"<<endl;
	}
}
