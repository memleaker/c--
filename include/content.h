#ifndef __CONTENT_H__
#define __CONTENT_H__

#include <string>

namespace c89 {

class Content {
public:
    Content(const std::string& filename, const std::string &content)
        : m_row(1), m_column(0), m_filename(filename), m_content(content.c_str()),
          line_start (content.c_str()) {}

public:
    /* c++ */
    Content& operator++()
    {
        if (*m_content == '\n') {
            m_row++;
            m_column=0;
            line_start = m_content+1;
        } else {
            m_column++;
        }

        m_content++;

        return *this;
    }

    const char* operator+(int len)
    {
        return m_content+len;
    }

    const char* operator-(int len)
    {
        return m_content-len;
    }

    int operator-(Content & c)
    {
        return this->m_content - c.m_content;
    }

    Content& operator+=(unsigned int len)
    {
        while (len-- > 0)
        {
            if (*m_content == '\n') {
                m_row++;
                m_column=0;
                line_start = m_content+1;
            } else {
                m_column++;
            } 

            m_content++;
        }

        return *this;
    }

    char operator*()
    {
        return *m_content;
    }

    bool operator!=(Content& c)
    {
        return c.Str() != m_content;
    }

    bool operator==(Content& c)
    {
        return c.Str() == m_content;
    }

    inline const char* Str(void) {return m_content;}
    inline const std::string& Filename(void) {return m_filename;}
    inline unsigned int Row(void) {return m_row;}
    inline unsigned int Column(void) {return m_column;}
    inline std::string Location(void) { return m_filename+":"+std::to_string(m_row)+":"+std::to_string(m_column) + " "; }
    inline std::string Currline(void) 
    {
         return std::string("\n\t\t") + std::to_string(m_row) + " | " + 
                std::string(line_start, FindNewLine(m_content)) + "\n";
    }

private:
    const char *FindNewLine(const char *str)
    {
        for (; *str != '\0'; str++)
        {
            if (*str == '\n') {
                return str;
            }
        }

        return str;
    }


private:
    unsigned int m_row;
    unsigned int m_column;
    std::string m_filename;
    const char *m_content;
    const char *line_start;
};

}

#endif