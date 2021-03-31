#include "desc.h"
#include <iostream>
#include <fstream>
#include <string>
#include <direct.h>  
#include <algorithm>
#include <map>

#include "xrescode_extensions_v3.pb.h"

FileGenerator::FileGenerator(const google::protobuf::FileDescriptorProto& fdesc)
    : m_fdesc(fdesc)
{
    
}


Tool::Tool()
{
    // TODO: 改从配置文件读
    m_filterSet.insert("pb_header_v3.proto");
    m_filterSet.insert("xresloader_ue.proto");
    m_filterSet.insert("xresloader.proto");
    m_filterSet.insert("xrescode_extensions_v3.proto");
    m_filterSet.insert("descriptor.proto");

    m_outns = "cfg";
}

void Tool::Do(const std::string& pbfile, const std::string& outpath)
{
    m_pbfilepath = pbfile;
    m_outpath = outpath;

    Init();
    ParseProto();
}

void Tool::Init()
{
    if (access(m_outpath.c_str(), 0) == -1)
    {
        mkdir(m_outpath.c_str()); 
    }
}

void Tool::ParseProto()
{
    google::protobuf::FileDescriptorSet fds;
    std::fstream fin;
    fin.open(m_pbfilepath, std::ios::in | std::ios::binary);
    if (!fin.is_open())
    {
        std::cout << "open " << m_pbfilepath << " failed" << std::endl;
        return;
    }
    fds.ParseFromIstream(&fin);
    for (auto& file : fds.file())
    {
        GenerateOneFile(file);
    }    

    GenReadConfCode();
}

void Tool::GenerateOneFile(const google::protobuf::FileDescriptorProto& fdesc)
{
    if (IsFilterFile(fdesc.name()))
    {
        return;
    }

    FileGenerator fg(fdesc);
    if (!WriteHeader(fg))
    {
        return;
    }

    std::string tabs;
    if (!fdesc.package().empty())
    {
        fg.m_content.append("namespace " + fdesc.package() + "\n");
        fg.m_content.append("{\n");
        tabs.push_back('\t');
    }

    std::cout << fg.m_fileName << std::endl;

    WriteEnums(fg, fdesc.enum_type(), tabs);

    for (auto& msg : fdesc.message_type())
    {
        if (fg.m_writeSet.count(msg.name())) continue;
        
        std::cout << "message: " << msg.name() << std::endl;
        WriteMessage(fg, fdesc.package(), msg, tabs);

        int len = msg.options().ExtensionSize(xrescode::loader);
        for (int i = 0; i < len; ++i)
        {
            auto loader = msg.options().GetExtension(xrescode::loader, i);
            if (!loader.file_path().empty() || !loader.file_list().empty())
            {
                // std::cout << "need gen parse config data code" << std::endl;
                m_readList.push_back(&msg);
                break;
            }
        }
    }

    std::cout << "-------------------writeSet [" << std::endl;
    for (auto& mn : fg.m_writeSet)
    {
        std::cout << mn << std::endl;
    }
    std::cout << "-------------------] writeSet" << std::endl;

    if (!fdesc.package().empty())
    {
        fg.m_content.append("}\n");
    }
    fg.m_content.append("\n");
    fg.m_content.append("#endif /* _" + fg.m_fileNameNoSuffix + "_H_ */\n");

    // std::cout << content << std::endl;
    std::ofstream fout;
    fout.open(m_outpath + "./" + fg.m_fileName, std::ios::out);
    fout.write(fg.m_content.c_str(), fg.m_content.size());
    fout.close();
}


bool Tool::WriteHeader(FileGenerator& fg)
{
    fg.m_fileName = fg.m_fdesc.name();
    ReplaceString(fg.m_fileName, ".proto", "");
    // auto p = fileName.find(".proto");
    // if (p == std::string::npos)
    // {
    //     std::cout << "error here 1" << std::endl;
    //     return false;
    // }

    // fileNameNoSuffix = fileName.substr(0, p);
    // fileName.replace(p, 6, ".h");

    fg.m_fileNameNoSuffix = fg.m_fileName;
    fg.m_fileName += ".h";

    std::transform(fg.m_fileNameNoSuffix.begin(), fg.m_fileNameNoSuffix.end(), fg.m_fileNameNoSuffix.begin()
        , [](unsigned char c) -> unsigned char { return std::toupper(c); });
    
    fg.m_content.append("#ifndef _" + fg.m_fileNameNoSuffix + "_H_\n");
    fg.m_content.append("#define _" + fg.m_fileNameNoSuffix + "_H_\n\n");

    fg.m_content.append("#include <stdint.h>\n");
    fg.m_content.append("#include <unordered_map>\n");
    fg.m_content.append("#include <string>\n\n");

    for (auto& dep : fg.m_fdesc.dependency())
    {
        // 写头Import 的头文件
        if (IsFilterFile(dep)) continue;
        std::string deph = dep;
        ReplaceString(deph, ".proto", ".h");
        fg.m_content.append("#include \"" + deph + "\"\n");
    }
    if (fg.m_fdesc.dependency_size()) fg.m_content.append("\n\n");

    return true;
}
void Tool::WriteMessage(FileGenerator& fg, const std::string& ns, const google::protobuf::DescriptorProto& msg, std::string tabs)
{
    WriteSubMsg(fg, ns, msg, tabs);

    fg.m_content.append(tabs + "struct " + msg.name() + "\n");
    fg.m_content.append(tabs + "{\n");
    WriteEnums(fg, msg.enum_type(), tabs + "\t");
    WriteNestedMessage(fg, ns.empty() ? msg.name() : ns + "::" + msg.name(), msg, tabs + "\t");
    WriteDataMembers(fg, ns, msg, tabs);
    fg.m_content.append(tabs + "};\n");

    fg.m_writeSet.insert(msg.name());
}

void Tool::WriteSubMsg(FileGenerator& fg, const std::string& ns, const google::protobuf::DescriptorProto& msg, std::string tabs)
{
    // 前置声明
    for (auto& field : msg.field())
    {
        if (google::protobuf::FieldDescriptorProto::TYPE_MESSAGE == field.type())
        {
            // 是否是内嵌类型
            uint32_t m = GetFieldFlag(msg, field);
            if (m & 1)
            {
                continue;
            }
            else 
            {
                std::string typeName = field.type_name();
                if (!typeName.empty() && typeName[0] == '.') 
                {
                    typeName = typeName.substr(1);
                }
                
                if (typeName.find('.') == std::string::npos && !fg.m_writeSet.count(typeName))
                {
                    // content.append(tabs + "struct " + GetTypeName(msg, field) + ";\n");
                    for (auto& _msg : fg.m_fdesc.message_type())
                    {
                        if (_msg.name() == typeName)
                        {
                            WriteMessage(fg, ns, _msg, tabs);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void Tool::WriteNestedMessage(FileGenerator& fg, const std::string& ns, const google::protobuf::DescriptorProto& msg, std::string tabs)
{
    std::set<std::string> s;
    int i = 0;
    while (s.size() < msg.nested_type_size() && ++i < 1000)
    {
        for (auto& nt : msg.nested_type())
        {
            if (s.count(nt.name())) continue;

            if (nt.options().map_entry()) 
            {
                s.insert(nt.name());
                continue;
            }

            bool writeNow = true;
            for (auto& field : nt.field())
            {
                int j = 0;
                for (; j < msg.nested_type_size(); ++j)
                {
                    auto& nt2 = msg.nested_type(j);
                    // std::cout << "A: " << GetTypeName(nt, field) << "   B: " << ns + "::" + nt2.name() << std::endl;
                    if (GetTypeName(nt, field) == ns + "::" + nt2.name())
                    {
                        break;
                    }
                }
                if (j < msg.nested_type_size() && !s.count(msg.nested_type(j).name()))
                {
                    writeNow = false;
                }
            }
            if (writeNow)
            {
                WriteMessage(fg, ns, nt, tabs);
                s.insert(nt.name());
            }
        }
    }
    if (i >= 1000)
    {
        std::cout << "Tool::WriteNestedMessage loop count more than 1000" << std::endl;
    }
}

void Tool::WriteDataMembers(FileGenerator& fg, const std::string& ns, const google::protobuf::DescriptorProto& msg, std::string tabs)
{
    std::string inner_ns = ns.empty() ? msg.name() + "::" : ns + "::" + msg.name() + "::";
    // 成员变量
    for (auto& field : msg.field())
    {
        std::string tn = GetTypeName(msg, field);
        // std::cout << "tn: " << tn << "  ns: " << inner_ns << std::endl; 
        std::string::size_type p1 = inner_ns.find("::");
        std::string::size_type p2 = tn.find("::");
        std::string::size_type moveHeadPos = std::string::npos;
        while (p1 != std::string::npos && p2 != std::string::npos)
        {
            if (p1 == p2 && inner_ns.substr(0, p1) == tn.substr(0, p2))
            {
                moveHeadPos = p1;
                p1 = inner_ns.find("::", p1 + 2);
                p2 = tn.find("::", p2 + 2);
            }
            else break;
        }
        if (moveHeadPos != std::string::npos)
        {
            tn = tn.substr(moveHeadPos+2);
        }
        fg.m_content.append(tabs + "\t" + tn + " " + field.name() + ";\n");
    }
}

void Tool::WriteEnums(FileGenerator& fg, const google::protobuf::RepeatedPtrField<google::protobuf::EnumDescriptorProto >& enums, std::string tabs)
{
    for (auto& e : enums)
    {
        fg.m_content.append(tabs + "enum " + e.name() + "\n");
        fg.m_content.append(tabs + "{\n");
        for (auto& v : e.value())
        {
            fg.m_content.append(tabs + "\t" + v.name() + " = " + std::to_string(v.number()) + ",\n");
        }
        fg.m_content.append(tabs + "};\n");
    }
}

std::string Tool::GetTypeName(const google::protobuf::DescriptorProto& msg, const google::protobuf::FieldDescriptorProto& field)
{
    switch (field.type())
    {
        case google::protobuf::FieldDescriptorProto::TYPE_DOUBLE:
            return "double";
        case google::protobuf::FieldDescriptorProto::TYPE_FLOAT:
            return "float";
        case google::protobuf::FieldDescriptorProto::TYPE_INT64:
        case google::protobuf::FieldDescriptorProto::TYPE_SFIXED64:
        case google::protobuf::FieldDescriptorProto::TYPE_SINT64:
            return "int64_t";
        case google::protobuf::FieldDescriptorProto::TYPE_UINT64:
        case google::protobuf::FieldDescriptorProto::TYPE_FIXED64:
            return "uint64_t";
        case google::protobuf::FieldDescriptorProto::TYPE_INT32:
        case google::protobuf::FieldDescriptorProto::TYPE_SFIXED32:
        case google::protobuf::FieldDescriptorProto::TYPE_SINT32:
            return "int32_t";
        case google::protobuf::FieldDescriptorProto::TYPE_FIXED32:
        case google::protobuf::FieldDescriptorProto::TYPE_UINT32:
            return "uint32_t";

        case google::protobuf::FieldDescriptorProto::TYPE_BOOL:
            return "bool";
        case google::protobuf::FieldDescriptorProto::TYPE_STRING:
        case google::protobuf::FieldDescriptorProto::TYPE_BYTES:
            return "std::string";   // 考虑共享内存，要改
        default: break;
    }


    const google::protobuf::DescriptorProto* nestedmsg = nullptr;
    uint32_t m = GetFieldFlag(msg, field, &nestedmsg);
    if (m & 2)
    {
        assert(nestedmsg->field_size() == 2);
        return "std::unordered_map<" + GetTypeName(*nestedmsg, nestedmsg->field(0)) + ", " + GetTypeName(*nestedmsg, nestedmsg->field(1)) + ">";
    }
    
    std::string typeName = field.type_name();
    if (!typeName.empty() && typeName[0] == '.')
    {
        typeName = typeName.substr(1);
    }
    ReplaceString(typeName, ".", "::");

    return typeName;
}

uint32_t Tool::GetFieldFlag(const google::protobuf::DescriptorProto& msg, const google::protobuf::FieldDescriptorProto& field, const google::protobuf::DescriptorProto** nested_msg)
{
    uint32_t m = 0;
    for (auto& nt : msg.nested_type())
    {
        if ("." + msg.name() + "." + nt.name() == field.type_name())
        {
            m |= 1;
            if (nt.options().map_entry())
            {
                m |= 2;
            }
            if (nested_msg) *nested_msg = &nt;
            return m;
        }
    }
    return m;
}

void Tool::ReplaceString(std::string& str, const std::string& subo, const std::string& subn)
{
    auto p = str.find(subo);
    while (p != std::string::npos)
    {
        str.replace(p, subo.size(), subn);
        p = str.find(subo, p + subn.size());
    }
}

bool Tool::IsFilterFile(const std::string& fileName)
{
    for (auto& str : m_filterSet)
    {
        if (fileName.find(str) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

void Tool::GenReadConfCode() const
{
    for (auto* msg : m_readList)
    {
        GenConfig(*msg);
    }
}

void Tool::GenConfig(const google::protobuf::DescriptorProto& msg) const
{

}
