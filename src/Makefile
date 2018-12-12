#MY_GCC_OPT = -g

netflow_converter: ../obj/data_loader.o ../obj/data_loader_bz.o ../obj/main.o ../obj/netflow_data_types.o ../obj/file_list.o ../obj/netflow_converter.o ../obj/ipfix_converter.o ../obj/ipfix_data_types.o ../obj/ipfix_converter_cache.o ../obj/ipfix_dictionary.o ../obj/ipfix_dict_iana.o ../obj/logger.o ../obj/options.o ../obj/filter_time.o ../obj/filter_data.o ../obj/netflow_mapper.o ../obj/ipfix_mapper.o
	g++ ../obj/data_loader.o -o ../netflow_converter ../obj/main.o ../obj/netflow_data_types.o ../obj/netflow_converter.o ../obj/ipfix_converter.o ../obj/ipfix_data_types.o ../obj/ipfix_converter_cache.o ../obj/ipfix_dictionary.o ../obj/ipfix_dict_iana.o ../obj/file_list.o ../obj/logger.o ../obj/options.o ../obj/data_loader_bz.o ../obj/filter_time.o ../obj/filter_data.o ../obj/netflow_mapper.o ../obj/ipfix_mapper.o -lbz2 -lpthread

../obj/netflow_data_types.o: converter/netflow_data_types.h converter/netflow_data_types.cpp
	g++ $(MY_GCC_OPT) -c converter/netflow_data_types.cpp -o ../obj/netflow_data_types.o
../obj/netflow_converter.o: converter/netflow_converter.h converter/netflow_converter.cpp
	g++ $(MY_GCC_OPT) -c converter/netflow_converter.cpp -o ../obj/netflow_converter.o

../obj/ipfix_converter.o: converter/ipfix/ipfix_converter.h converter/ipfix/ipfix_converter.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/ipfix_converter.cpp -o ../obj/ipfix_converter.o
../obj/ipfix_data_types.o: converter/ipfix/ipfix_data_types.h converter/ipfix/ipfix_data_types.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/ipfix_data_types.cpp -o ../obj/ipfix_data_types.o
../obj/ipfix_converter_cache.o: converter/ipfix/ipfix_converter_cache.h converter/ipfix/ipfix_converter_cache.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/ipfix_converter_cache.cpp -o ../obj/ipfix_converter_cache.o --std=c++11
../obj/ipfix_dictionary.o: converter/ipfix/dict/ipfix_dictionary.h converter/ipfix/dict/ipfix_dictionary.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/dict/ipfix_dictionary.cpp -o ../obj/ipfix_dictionary.o --std=c++11
../obj/ipfix_dict_iana.o: converter/ipfix/dict/iana/ipfix_dict_iana.h converter/ipfix/dict/iana/ipfix_dict_iana.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/dict/iana/ipfix_dict_iana.cpp -o ../obj/ipfix_dict_iana.o --std=c++11

../obj/data_loader.o: data/data_loader.h data/data_loader.cpp
	g++ $(MY_GCC_OPT) -c data/data_loader.cpp -o ../obj/data_loader.o
../obj/data_loader_bz.o: data/data_loader_bz.h data/data_loader_bz.cpp
	g++ $(MY_GCC_OPT) -c data/data_loader_bz.cpp -o ../obj/data_loader_bz.o
../obj/file_list.o: data/file_list.h data/file_list.cpp
	g++ $(MY_GCC_OPT) -c data/file_list.cpp -o ../obj/file_list.o

../obj/logger.o: util/logger.h util/logger.cpp
	g++ $(MY_GCC_OPT) -c util/logger.cpp -o ../obj/logger.o
../obj/options.o: util/options.h util/options.cpp
	g++ $(MY_GCC_OPT) -c util/options.cpp -o ../obj/options.o

../obj/filter_time.o: filter/filter_time.h filter/filter_time.cpp
	g++ $(MY_GCC_OPT) -c filter/filter_time.cpp -o ../obj/filter_time.o
../obj/filter_data.o: filter/nf_filter_data.h filter/nf_filter_data.cpp
	g++ $(MY_GCC_OPT) -c filter/nf_filter_data.cpp -o ../obj/filter_data.o

../obj/netflow_mapper.o: mapper/netflow_converter_mapper.h mapper/netflow_converter_mapper.cpp
	g++ $(MY_GCC_OPT) -c mapper/netflow_converter_mapper.cpp -o ../obj/netflow_mapper.o --std=c++11
../obj/ipfix_mapper.o: converter/ipfix/mapper/ipfix_mapper.h converter/ipfix/mapper/ipfix_mapper.cpp
	g++ $(MY_GCC_OPT) -c converter/ipfix/mapper/ipfix_mapper.cpp -o ../obj/ipfix_mapper.o --std=c++11

../obj/main.o: main.cpp
	g++ $(MY_GCC_OPT) -c main.cpp -o ../obj/main.o

clean:
	rm -f ../obj/*

rebuild: clean netflow_converter
