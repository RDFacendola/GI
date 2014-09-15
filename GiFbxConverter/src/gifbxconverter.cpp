/// GiFbxConverter

/// v.0.1 - Import a fbx mesh and export it sanitized and triangulated (-triangulate)




#include <exception>
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "..\include\fbx.h"

using namespace ::std;
using namespace ::gi_lib;

const char kCommandToken = '-';
const string kGlobalCommand = "$Global";				// Commandless arguments
const string kHelpCommand = "-?";						// Halp!
const string kTriangulateCommand = "-triangulate";		// Triangulate
const string kRemap = "-remap";							// Remap the mesh
const string kDds = "-dds";								// Converts all the texture in dds format
const string kOutputCommand = "-o";						// Mandatory
const string kInputCommand = "-i";						// Mandatory

using CommandMap = multimap < string, vector<string> >;

void ShowHelp();

void ShowUsage();

void Run(CommandMap & commands);

CommandMap ParseCommands(int argc, char* argv[]);

void main(int argc, char* argv[]){

	auto commands = ParseCommands(argc, argv);

	CommandMap::iterator it;

	if (commands.find(kHelpCommand) != commands.end()){

		//Halp!
		ShowHelp();

	}
	else if((it = commands.find(kInputCommand)) == commands.end() ||
			 it->second.size() == 0 ||
			(it = commands.find(kOutputCommand)) == commands.end() ||
			 it->second.size() == 0){

		//Mandatory parameters missing!
		ShowUsage();

	}
	else{

		//Run
		Run(commands);
		
	}

#ifdef _DEBUG

	cin.get();

#endif
		
}

CommandMap ParseCommands(int argc, char* argv[]){

	CommandMap parameters;

	int command_index = -1;	//Last command's index
	string key;				//Last command's name
	vector<string> values;	//Last command's values

	//Parameters beginning with a "-" are keys, the rest are values...
	for (int arg_index = 0; arg_index < argc; ++arg_index){

		if (argv[arg_index][0] == kCommandToken){

			// Add a command_index entry inside the parameter map
			key = command_index > -1 ?
				argv[command_index] :
				kGlobalCommand;

			parameters.insert(make_pair(key, values));

			values.clear();

			command_index = arg_index;

		}
		else
		{

			// Add a new value to the previous command
			values.push_back(argv[arg_index]);

		}

	}

	//Add the last values...
	key = command_index > -1 ?
		argv[command_index] :
		kGlobalCommand;

	parameters.insert(make_pair(key, values));

	return parameters;

}

void ShowHelp(){

	// Intro
	cout << std::endl;

	cout << "Gi Fbx Converter utility." << std::endl;
	cout << "This command performs conversion of FBX files." << std::endl;

	cout << std::endl;

	// Usage
	ShowUsage();

	cout << "[input file]: complete path of the file to import (extension included)." << std::endl
		<< "[output file]: complete path of the file to export (extension included)." << std::endl
		<< "[options]: " << std::endl;

	// Commands
	cout << std::endl;

	//cout << kTriangulateCommand << ": Triangulate the mesh." << std::endl;
	cout << kRemap << ": Performs a per-vertex remapping of mesh attributes. " << std::endl;
	cout << kDds << ": Convert each texture addressed by the mesh in dds format. (Note \"texconv\" must be recognized as command)" << std::endl;

}

void ShowUsage(){

	cout << std::endl;

	cout << "Usage: -i [input file] -o [output file] [options]" << std::endl
		 << "Type -? to access help " << std::endl;

	cout << std::endl;

}

void Run(CommandMap & commands){


	FbxScene * scene = nullptr;

	try{

		auto & fbx = FBX::GetInstance();

		// IMPORT
		cout << "Importing..." << std::endl;

		auto input = commands.find(kInputCommand);

		scene = fbx.Import(input->second.front());

		//EXECUTION

		//if (commands.find(kTriangulateCommand) != commands.end()){

			cout << "Triangulating (this could take a couple of minutes)..." << std::endl;

			fbx.Triangulate(*scene);

		//}

		if (commands.find(kRemap) != commands.end()){

			cout << "Re-mapping mesh attributes..." << std::endl;

			fbx.RemapAttributes(*scene);

		}
		
		if (commands.find(kDds) != commands.end()){

			cout << "Converting textures..." << std::endl;



		}

		// EXPORT

		cout << "Exporting to FBX..." << std::endl;

		auto output = commands.find(kOutputCommand);

		fbx.Export(*scene, output->second.front());

		// YAY!

		cout << "Done!" << std::endl;

	}
	catch (std::exception & e){

		//Darn...
		cout << e.what() << std::endl;

	}

	//Cleanup
	if (scene != nullptr){

		scene->Destroy();

	}

}