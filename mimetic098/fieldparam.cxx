#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wuseless-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wswitch-default"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsuggest-override"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wold-style-cast"

#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"


/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: fieldparam.cxx,v 1.3 2008-10-07 11:06:25 tat Exp $
 ***************************************************************************/
#include <mimetic098/fieldparam.h>
#include <mimetic098/utils.h>

namespace mimetic
{

using namespace std;

FieldParam::FieldParam()
{
}

FieldParam::FieldParam(const string& lpv)
{
    string::const_iterator bit = lpv.begin(), eit = lpv.end();
    for( ; bit != eit; ++bit)
    {
        if(*bit == '=')
        {
            string n(lpv.begin(), bit), v(++bit, eit);
            m_name.assign (remove_external_blanks(n));
            m_value = remove_dquote(remove_external_blanks(v));
            break;
        }
    }
}

FieldParam::FieldParam(const string& n, const string& v)
{    
    name(n);    
    value(v);
}

const istring& FieldParam::name() const
{    
    return m_name;                
}

const string& FieldParam::value() const
{    
    return m_value;                
}

void FieldParam::name(const string& n)
{    
    m_name.assign(n);
}

void FieldParam::value(const string& v)
{    
    m_value = v;
}

ostream& operator<<(ostream& os, const FieldParam& p)
{
    os << p.name() << "=";
    const string& val = p.value();
    if(val.find_first_of("()\\<>\"@,;:/[]?=") != std::string::npos)
        return os << "\"" << val << "\"";
    else
        return os << val;
}

}
