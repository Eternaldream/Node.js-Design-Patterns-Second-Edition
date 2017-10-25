#include <iostream>
using namespace std;

size_t _strlen(const char * str)
{
  size_t i = 0;
  while(*str++)
  {
    i++;
  }
  return i;
}

char *_strcpy(char *dest, const char *src)
{
  size_t length = _strlen(src);
  // 如果有dest，先释放dest内存
  if (dest)
  {
    delete[] dest;
  }
  dest = new char[length + 1];
  for (int i = 0; i < length; ++i)
  {
    dest[i] = src[i];
  }
  dest[length] = '\0';
  return dest;
}

char *_strcat(char *dest, const char *src)
{
  size_t length_dest = _strlen(dest);
  size_t length_src = _strlen(src);
  char *str_tmp = new char[length_src + length_dest + 1];
  for (int i = 0; i < length_dest; ++i)
  {
    str_tmp[i] = dest[i];
  }
  for (int i = 0; i < length_src; ++i)
  {
    str_tmp[i + length_dest] = src[i];
  }
  str_tmp[length_dest + length_src] = '\0';
  delete[] dest;
  dest = str_tmp;
  return dest;
}

class cstring
{
public:
  cstring()
  {
    c_str = new char[1];
    *c_str = '\0';
    c_len = 0;
  }

  cstring(char character)
  {
    c_str = new char[2];
    c_str[0] = character;
    c_str[1] = '\0';
    c_len = 1;
  }

  cstring(const char * str)
  {
    if (str != NULL)
    {
      c_len = _strlen(str);
      c_str = new char[c_len + 1];
      for (int i = 0; i < c_len; ++i)
      {
        c_str[i] = str[i];
      }
      c_str[c_len] = '\0';
    }
  }
  cstring(const cstring &strObj)
  {
    char *str = strObj.c_str;
    c_len = strObj.c_len;
    c_str = new char[c_len + 1];
    for (int i = 0; i < c_len; ++i)
    {
      c_str[i] = str[i];
    }
    c_str[c_len] = '\0';
  }
  cstring(cstring&& strObj)
  {
    char *str = strObj.c_str;
    c_len = strObj.c_len;
    c_str = new char[c_len + 1];
    for (int i = 0; i < c_len; ++i)
    {
      c_str[i] = str[i];
    }
    c_str[c_len] = '\0';
  }

  size_t size() const
  {
    return c_len;
  }

  bool isEmpty() const
  {
    return c_len == 0;
  }

  void empty()
  {
    c_len = 0;
    delete[] c_str;
    c_str = new char[1];
    *c_str = '\0';
  }

  cstring& concate(const cstring& strObj)
  {
    char *str = _strcat(this->c_str, strObj.c_str);
    this->c_str = str;
    this->c_len += strObj.c_len;
    return *this;
  }

  cstring& concate(const char * str)
  {
    char *str_tmp = _strcat(this->c_str, str);
    this->c_str = str_tmp;
    this->c_len += _strlen(str);
    return *this;
  }

  char * get_str() const
  {
    return c_str;
  }

  ~cstring()
  {
    if (c_str != NULL)
    {
      delete[] c_str;
    }
    c_len = 0;
  }
  
private:
  char *c_str;
  int c_len;
};

int main(int argc, char const *argv[])
{
  cstring str1;
  cout << "测试cstring()的size：" << str1.size() << endl;
  cout << "测试isEmpty()：" << str1.isEmpty() << endl;
  cstring str2('c');
  cout << "测试cstring(char)的size：" << str2.size() << endl;
  cout << "测试isEmpty()：" << str2.isEmpty() << endl;
  char *tmp = "c++";
  cstring str3(tmp);
  cout << "测试cstring(const char *)的size：" << str3.size() << endl;
  cstring str4(str3);
  cout << "测试cstring(const cstring&)的size：" << str4.size() << endl;
  cstring str5(cstring("python"));
  cout << "测试cstring(cstring&&)的size：" << str5.size() << endl;
  cout << "测试get_str()：" << str5.get_str() << endl;
  str5.empty();
  cout << "测试empty()，empty后的length为：" << str5.size() << endl;
  str2.concate(str3);
  cout << "测试cstring& concate(const cstring&)，连接str2和str3后的字符串预期为cc++：" << str2.get_str() << endl;
  return 0;
}