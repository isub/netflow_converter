MY_GCC_OPT = -Wall -Wextra --std=c++11 $(CXXFALGS)
export MY_GCC_OPT

MY_DBG_OPT = "-g -DDEBUG"
MY_RLS_OPT = "-O3"

OBJ = obj/nfc_dict.o obj/data_loader.o obj/data_loader_bz.o obj/main.o obj/netflow_data_types.o obj/file_list.o obj/netflow_converter.o obj/ipfix_converter.o obj/ipfix_data_types.o obj/ipfix_converter_cache.o obj/ipfix_dictionary.o obj/ipfix_dict_iana.o obj/logger.o obj/options.o obj/filter_time.o obj/filter_data.o obj/netflow_mapper.o obj/nfv9_converter.o obj/nfv9_data_types.o obj/nfv9_converter_cache.o obj/nfv9_dict.o obj/conf.tab.o obj/lex.conf.o obj/nfc_output_template.o

netflow_converter: $(OBJ)
	g++ -o ../netflow_converter $(OBJ) -lbz2 -lpthread

obj/netflow_data_types.o: converter/netflow_data_types.h converter/netflow_data_types.cpp
	g++ $(MY_GCC_OPT) -c converter/netflow_data_types.cpp -o obj/netflow_data_types.o
obj/netflow_converter.o: converter/netflow_converter.h converter/netflow_converter.cpp
	g++ $(MY_GCC_OPT) -c converter/netflow_converter.cpp -o obj/netflow_converter.o
obj/nfc_dict.o: converter/nfc_dict.h converter/nfc_dict.cpp
	g++ $(MY_GCC_OPT) -c converter/nfc_dict.cpp -o obj/nfc_dict.o

obj/ipfix_converter.o: converter/ipfix/ipfix_converter.h converter/ipfix/ipfix_converter.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/ipfix_converter.cpp -o obj/ipfix_converter.o
obj/ipfix_data_types.o: converter/ipfix/data_types/ipfix_data_types.h converter/ipfix/data_types/ipfix_data_types.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/data_types/ipfix_data_types.cpp -o obj/ipfix_data_types.o
obj/ipfix_converter_cache.o: converter/ipfix/ipfix_converter_cache.h converter/ipfix/ipfix_converter_cache.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/ipfix_converter_cache.cpp -o obj/ipfix_converter_cache.o
obj/ipfix_dictionary.o: converter/ipfix/dict/ipfix_dictionary.h converter/ipfix/dict/ipfix_dictionary.cpp obj/nfc_dict.o
	g++ $(MY_GCC_OPT) -c converter/ipfix/dict/ipfix_dictionary.cpp -o obj/ipfix_dictionary.o
obj/ipfix_dict_iana.o: converter/ipfix/dict/iana/ipfix_dict_iana.h converter/ipfix/dict/iana/ipfix_dict_iana.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/dict/iana/ipfix_dict_iana.cpp -o obj/ipfix_dict_iana.o

obj/nfv9_converter.o: converter/nfv9/nfv9_converter.h converter/nfv9/nfv9_converter.cpp
	g++ $(MY_GCC_OPT) -c converter/nfv9/nfv9_converter.cpp -o obj/nfv9_converter.o
obj/nfv9_data_types.o: converter/nfv9/data_types/nfv9_data_types.h converter/nfv9/data_types/nfv9_data_types.cpp
	g++ $(MY_GCC_OPT) -c converter/nfv9/data_types/nfv9_data_types.cpp -o obj/nfv9_data_types.o
obj/nfv9_converter_cache.o: converter/nfv9/cache/nfv9_converter_cache.h converter/nfv9/cache/nfv9_converter_cache.cpp
	g++ $(MY_GCC_OPT) -c converter/nfv9/cache/nfv9_converter_cache.cpp -o obj/nfv9_converter_cache.o
obj/nfv9_dict.o: converter/nfv9/dict/nfv9_dict.h converter/nfv9/dict/nfv9_dict.cpp obj/nfc_dict.o
	g++ $(MY_GCC_OPT) -c converter/nfv9/dict/nfv9_dict.cpp -o obj/nfv9_dict.o

obj/data_loader.o: data/data_loader.h data/data_loader.cpp
	g++ $(MY_GCC_OPT) -c data/data_loader.cpp -o obj/data_loader.o
obj/data_loader_bz.o: data/data_loader_bz.h data/data_loader_bz.cpp
	g++ $(MY_GCC_OPT) -c data/data_loader_bz.cpp -o obj/data_loader_bz.o
obj/file_list.o: data/file_list.h data/file_list.cpp
	g++ $(MY_GCC_OPT) -c data/file_list.cpp -o obj/file_list.o

obj/logger.o: util/logger.h util/logger.cpp
	g++ $(MY_GCC_OPT) -c util/logger.cpp -o obj/logger.o
obj/options.o: util/options.h util/options.cpp
	g++ $(MY_GCC_OPT) -c util/options.cpp -o obj/options.o

obj/filter_time.o: filter/filter_time.h filter/filter_time.cpp
	g++ $(MY_GCC_OPT) -c filter/filter_time.cpp -o obj/filter_time.o
obj/filter_data.o: filter/nf_filter_data.h filter/nf_filter_data.cpp
	g++ $(MY_GCC_OPT) -c filter/nf_filter_data.cpp -o obj/filter_data.o

obj/netflow_mapper.o: mapper/netflow_converter_mapper.h mapper/netflow_converter_mapper.cpp
	g++ $(MY_GCC_OPT) -c mapper/netflow_converter_mapper.cpp -o obj/netflow_mapper.o

obj/main.o: main.cpp
	g++ $(MY_GCC_OPT) -c main.cpp -o obj/main.o

config/lex.conf.cpp: config/conf.l
	flex -Pconf -o config/lex.conf.cpp config/conf.l
config/conf.tab.cpp: config/conf.y
	bison --name-prefix=conf --defines --output-file=config/conf.tab.cpp config/conf.y
obj/conf.tab.o: config/conf.tab.cpp
	g++ $(MY_GCC_OPT) -o obj/conf.tab.o -c config/conf.tab.cpp
obj/lex.conf.o: config/lex.conf.cpp
	g++ $(MY_GCC_OPT) -o obj/lex.conf.o -c config/lex.conf.cpp

obj/nfc_output_template.o: converter/nfc_output_template.h converter/nfc_output_template.cpp
	g++ $(MY_GCC_OPT) -o obj/nfc_output_template.o -c converter/nfc_output_template.cpp

clean:
	rm -f obj/*

rebuild: clean netflow_converter

install:
	cp ../netflow_converter /usr/local/bin

debug:
	export CXXFALGS=$(MY_DBG_OPT) && $(MAKE) -f Makefile rebuild

release:
	export CXXFALGS=$(MY_RLS_OPT) && $(MAKE) -f Makefile rebuild