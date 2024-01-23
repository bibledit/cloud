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

/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: message.cxx,v 1.4 2008-10-07 11:06:25 tat Exp $
 ***************************************************************************/
#include <fstream>
#include <cstdlib>
#include <mimetic098/message.h>
#include <mimetic098/contenttype.h>
#include <mimetic098/utils.h>

namespace mimetic
{

using namespace std;
Attachment::Attachment(const std::string& fqn)
{
    set(fqn, ContentType("application","octet-stream"),Base64::Encoder());
}

Attachment::Attachment(const std::string& fqn, const ContentType& ct)
{
    set(fqn, ct, Base64::Encoder());
}


TextEntity::TextEntity()
{
    header().contentType("text/unknown");
}

TextEntity::TextEntity(const string& text)
{
    m_header.contentType("text/unknown");
    m_body.assign(text);
}

TextEntity::TextEntity(const string& text, const string& charset)
{
    ContentType ct("text", "unknown");
    ct.paramList().push_back(ContentType::Param("charset", charset));
    m_header.contentType(ct);
    m_body.assign(text);
}

TextPlain::TextPlain(const string& text)
: TextEntity(text)
{
    m_header.contentType("text/plain");
}

TextPlain::TextPlain(const string& text, const string& charset)
: TextEntity(text,charset)
{
    m_header.contentType("text/plain");
}


TextEnriched::TextEnriched(const string& text)
: TextEntity(text)
{
    m_header.contentType("text/enriched");
}
TextEnriched::TextEnriched(const string& text, const string& charset)
: TextEntity(text,charset)
{
    m_header.contentType("text/enriched");
}


MultipartEntity::MultipartEntity()
{
    ContentType::Boundary boundary;
    ContentType ct("multipart", "unknown");
    ct.paramList().push_back(ContentType::Param("boundary", boundary));
    m_header.contentType(ct);
}

MultipartMixed::MultipartMixed()
{
    ContentType::Boundary boundary;
    ContentType ct("multipart", "mixed");
    ct.paramList().push_back(ContentType::Param("boundary", boundary));
    m_header.contentType(ct);
}

MultipartParallel::MultipartParallel()
{
    ContentType::Boundary boundary;
    ContentType ct("multipart", "parallel");
    ct.paramList().push_back(ContentType::Param("boundary", boundary));
    m_header.contentType(ct);
}

MultipartAlternative::MultipartAlternative()
{
    ContentType::Boundary boundary;
    ContentType ct("multipart", "alternative");
    ct.paramList().push_back(ContentType::Param("boundary", boundary));
    m_header.contentType(ct);
}

MultipartDigest::MultipartDigest()
{
    ContentType::Boundary boundary;
    ContentType ct("multipart", "digest");
    ct.paramList().push_back(ContentType::Param("boundary", boundary));
    m_header.contentType(ct);
}

ApplicationOctStream::ApplicationOctStream()
{
    m_header.contentType("application/octet-stream");
}

MessageRfc822::MessageRfc822(const MimeEntity& me)
: m_me(me)
{
    m_header.contentType("message/rfc822");
}

ostream& MessageRfc822::write(ostream& os,const char* eol) const
{
    MimeEntity::write(os);
    return os << m_me;
}

string ApplicationOctStream::type() const
{
    return m_header.contentType().param("type");
}

void ApplicationOctStream::type(const string& type)
{
    ContentType ct = m_header.contentType();
    ct.param("type",type);
    m_header.contentType(ct);
}

uint ApplicationOctStream::padding() const
{
    return utils::str2int(m_header.contentType().param("padding"));
}

void ApplicationOctStream::padding(unsigned int n)
{
    ContentType ct = m_header.contentType();
    ct.param("padding", utils::int2str(n));
    m_header.contentType(ct);
}


}
