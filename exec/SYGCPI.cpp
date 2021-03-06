#include <emp-tool/emp-tool.h>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include "sygc/SYGC_config.h"
#include "sygc/program_interface.h"
#include "sygc/program_interface_sh.h"
#include "exec/exec_common.h"
#include "exec/unit_test.h"

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv) {
	int party, port;
	string netlist_address;
	string server_ip;
	int program;
	
	Timer T;
	memMeter M;
	T.start();
	
	po::options_description desc{"Unit test for SYGC programming interface \nAllowed options"};
	desc.add_options()  //
	("help,h", "produce help message")  //
	("party,k", po::value<int>(&party)->default_value(1), "party id: 1 for garbler, 2 for evaluator")  //
	("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
	("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"), "server's IP.")  //
	("sh", "semi-honest setting (default is malicious)") ;
	
	po::variables_map vm;
	try {
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
		po::store(parsed, vm);
		if (vm.count("help")) {
			cout << desc << endl;
			return 0;
		}
		po::notify(vm);
	}catch (po::error& e) {
		cout << "ERROR: " << e.what() << endl << endl;
		cout << desc << endl;
		return -1;
	}

	srand(time(NULL));
		
	NetIO* io = new NetIO(party==ALICE ? nullptr:server_ip.c_str(), port, true);
	io->set_nodelay();
	
	SYGCPI_SH* TGPI_SH;
	SYGCPI* TGPI; 
	
	if (vm.count("sh")){
		cout << "testing program interface in semi-honest setting" << endl;
		TGPI_SH = new SYGCPI_SH(io, party);
		io->flush();
		unit_test(TGPI_SH);		
	}
	else {
		cout << "testing program interface in malicious setting" << endl;
		TGPI = new SYGCPI(io, party, 100000, 100000);
		io->flush();
		unit_test(TGPI);
	}
	
	delete io;
	
	T.print("end");
	M.print("end");
	
	return 0;
}
