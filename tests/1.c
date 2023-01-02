int main (int argc, char **argv)
{
	char a = '\xff';
	char a1 = '\777';
	char a2 = '\0';
	char a3 = '\xffab';
	char a4 = '\n';
	char a5 = ' ';

	const char *str="Hello World!\"xxx\"\'\';+-*/";

	int i=0;
	long l = 111111L;
	long long ll = 12874892378426837462LL;
	unsigned long long ulll =0777LLU;
	unsigned long long ulll1 =0XFFFFabcdefULL;

	float f = 1.2345;
	float f1 = .01234F;
	double d=1.2345e3;
	long double ld=345.12345678L;

	printf("%s\n",str);

	return 0;
}
