#include<string>
#include<iostream>
#include<fstream>
#include<vector>
#include<sstream>
#include<algorithm>
#include<iterator>

namespace hsaosl{
std::vector<std::vector<std::string>> srcVec(1,std::vector<std::string>(0));

static void loadFile(std::string fileName){
	std::ifstream file(fileName);
	std::string line;
	char space = ' ';
	if(file.is_open()){
		while(getline(file, line)){
/*			int j=0;
			std::cout<<line<<std::endl;
			std::string l;
			for(int i=0;i<line.size();i++){
				if(line[i] != space && line[i] != '\t'){
					l += line[i];
				}else{
					if(l.size() > 0){
						srcVec.push_back(l);
					}
					l = "";
				}
			}
*/
		if(line[0] != '#'){
		std::istringstream iss(line);
		std::vector<std::string> src;
		std::copy(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(),
			std::back_inserter(src));
		srcVec.push_back(src);
		}}
		file.close();
	}
	for(int i=0;i<srcVec.size();i++){
		for(int j=0;j<srcVec[i].size();j++){
		std::cout<<srcVec[i][j]<<" ";
		}
		std::cout<<std::endl;
	}
}

}

int main(int argc, char **argv){
	if(argc != 2){
		std::cout<<"Usage: hsaosl <filename>.oso"<<std::endl;
		exit(0);
	}
	std::string fileName(argv[1]);
	std::cout<<fileName<<std::endl;
	int len = fileName.length();
	std::string ext(".sos");
	for(int i=1;i<4;i++){
		ext[i] = fileName[len-i];
	}
	if(!ext.compare(".oso")){
	std::cout<<ext<<std::endl;
	hsaosl::loadFile("phongbsdf.oso");
	}
}

