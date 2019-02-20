#ifndef __NF_FILTER_DATA_H__
#define __NF_FILTER_DATA_H__

#include <stdint.h>
#include <string>

#define NF_DATA_FILTER_FLOW_START "flowStart"
#define NF_DATA_FILTER_FLOW_END   "flowEnd"

#define NF_DATA_FILTER_SRC_IPV4   "sourceIPv4Address"
#define NF_DATA_FILTER_DST_IPV4   "destinationIPv4Address"

bool nf_filter_data( const std::string &p_strFieldName, uint64_t &p_ui64Data, uint32_t p_ui32Divider );

#endif /* __NF_FILTER_DATA_H__ */
