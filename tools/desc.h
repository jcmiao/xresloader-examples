#ifndef _DESC_H_
#define _DESC_H_

#include <string>
#include <list>
#include <set>
#include "descriptor.pb.h"

struct FileGenerator
{
    FileGenerator(const google::protobuf::FileDescriptorProto& fdesc);

    const google::protobuf::FileDescriptorProto& m_fdesc;
    std::string m_content;
    std::string m_fileName;
    std::string m_fileNameNoSuffix;
    std::set<std::string> m_writeSet;
};

class Tool
{
public:
    Tool();
    void Do(const std::string& pbfile, const std::string& outpath);

private:
    void Init();
    void ParseProto();
    void GenerateOneFile(const google::protobuf::FileDescriptorProto& fdesc);

    bool WriteHeader(FileGenerator& fg);
    void WriteMessage(FileGenerator& fg, const std::string& ns, const google::protobuf::DescriptorProto& msg, std::string tabs);
    void WriteSubMsg(FileGenerator& fg, const std::string& ns, const google::protobuf::DescriptorProto& msg, std::string tabs);
    void WriteNestedMessage(FileGenerator& fg, const std::string& ns, const google::protobuf::DescriptorProto& msg, std::string tabs);
    void WriteDataMembers(FileGenerator& fg, const std::string& ns, const google::protobuf::DescriptorProto& msg, std::string tabs);
    void WriteEnums(FileGenerator& fg, const google::protobuf::RepeatedPtrField<google::protobuf::EnumDescriptorProto >& enums, std::string tabs);

    std::string GetTypeName(const google::protobuf::DescriptorProto& msg, const google::protobuf::FieldDescriptorProto& field);
    uint32_t GetFieldFlag(const google::protobuf::DescriptorProto& msg, const google::protobuf::FieldDescriptorProto& field, const google::protobuf::DescriptorProto** nested_msg = nullptr);
    void ReplaceString(std::string& str, const std::string& subo, const std::string& subn);
    bool IsFilterFile(const std::string& fileName);

    // 读配置
    void GenReadConfCode() const;
    void GenConfig(const google::protobuf::DescriptorProto& msg) const;

private:
    std::string m_pbfilepath;
    std::string m_outpath;
    std::string m_outns;

    std::set<std::string> m_filterSet;

    std::list<const google::protobuf::DescriptorProto*> m_readList;
};

#endif /* _DESC_H_ */