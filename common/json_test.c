

#include "daemon_pub.h"
#include "hwmon_common.h"

#include "cJSON.h"


#ifdef _MSC_VER
#include <Windows.h>
void utf8_to_gbk(const char* utf8, char* gbk)
{
	const int maxlen = 256;
	wchar_t unicode_str[maxlen];
	int outlen = MultiByteToWideChar(CP_UTF8, 0, utf8, strlen(utf8), unicode_str, maxlen);
	outlen = WideCharToMultiByte(CP_ACP, 0, unicode_str, outlen, gbk, 256, NULL, NULL);
	gbk[outlen] = '\0';
}
#else // linux
void utf8_to_gbk(const char* utf8, char* gbk)
{
	strcpy(gbk, utf8);
}
#endif


static cJSON *parse_file(const char *filename)
{
    cJSON *parsed = NULL;
    char *content = read_file(filename);

    parsed = cJSON_Parse(content);

    if (content != NULL)
    {
        free(content);
    }

    return parsed;
}

#define CHECK_JSON_FILE
#ifndef CHECK_JSON_FILE
/*
{
  "name": "spring",
  "address": "北京",
  "age": 30,
  "value1": [[23, 43, -2.3, 6.7, 90],
             [-9, -19, 10, 2],
             [-5, -55]],
  "value2": [13.3, 1.9, 2.10],
  
  "bei_jing": {
    "address": "海淀",
    "car": false,
    "cat": true
  },
  "shan_dong": {
    "address": "济南",
    "value1": [{"ji_nan": "趵突泉"}, {"tai_an": "泰山"}]
  }
}

*/

int cli_json_test(int argc, char **argv)
{
	char *json = NULL;

    json = read_file("sample.json");
	if ((json == NULL) || (json[0] == '\0') || (json[1] == '\0')) {
		fprintf(stderr, "file content is null\n");
		return -1;
	}
 
	cJSON* items = cJSON_Parse(json);
	if (items == NULL) {
		fprintf(stderr, "pasre json file fail\n");
		return -1;
	}
    printf("top size %d \n", cJSON_GetArraySize(items));    
 
	char* printed_json = cJSON_PrintUnformatted(items);
	if (printed_json == NULL) {
		fprintf(stderr, "print json fail\n");
		return -1;
	}
	printf("%s\n\n", printed_json);
 
	cJSON*  item = cJSON_GetObjectItem(items, "name");
	fprintf(stdout, "key: %s, value: %s\n", "name", item->valuestring);
 
	char gbk[256];
	item = cJSON_GetObjectItem(items, "address");
	utf8_to_gbk(item->valuestring, gbk);
	fprintf(stdout, "key: %s, value: %s\n", "address", gbk);
 
	item = cJSON_GetObjectItem(items, "age");
	fprintf(stdout, "key: %s, value: %d\n", "age", item->valueint);
 
	item = cJSON_GetObjectItem(items, "value1");
	int size = cJSON_GetArraySize(item);
	for (int i = 0; i < size; ++i) {
		cJSON* tmp = cJSON_GetArrayItem(item, i);
		int len = cJSON_GetArraySize(tmp);
		fprintf(stdout, "key: %s:", "value1");
		for (int j = 0; j < len; ++j) {
			cJSON* tmp2 = cJSON_GetArrayItem(tmp, j);
			fprintf(stdout, " %f,", tmp2->valuedouble);
		}
		fprintf(stdout, "\n");
	}
 
	item = cJSON_GetObjectItem(items, "value2");
	size = cJSON_GetArraySize(item);
	fprintf(stdout, "key: %s:", "value2");
	for (int i = 0; i < size; ++i) {
		cJSON* tmp = cJSON_GetArrayItem(item, i);
		fprintf(stdout, " %f,", tmp->valuedouble);
	}
	fprintf(stdout, "\n");
 
	item = cJSON_GetObjectItem(items, "bei_jing");
	cJSON* tmp = cJSON_GetObjectItem(item, "address");
	utf8_to_gbk(tmp->valuestring, gbk);
	fprintf(stdout, "key: %s, value: %s\n", "address", gbk);
	tmp = cJSON_GetObjectItem(item, "car");
	fprintf(stdout, "key: %s, value: %d\n", "car", tmp->valueint);
	tmp = cJSON_GetObjectItem(item, "cat");
	fprintf(stdout, "key: %s, value: %d\n", "cat", tmp->valueint);
 
	item = cJSON_GetObjectItem(items, "shan_dong");
	tmp = cJSON_GetObjectItem(item, "address");
	utf8_to_gbk(tmp->valuestring, gbk);
	fprintf(stdout, "key: %s, value: %s\n", "address", gbk);
	tmp = cJSON_GetObjectItem(item, "value1");
	size = cJSON_GetArraySize(tmp);
	for (int i = 0; i < size; ++i) {
		char* names[2] = { "ji_nan", "tai_an" };
		cJSON* tmp2 = cJSON_GetArrayItem(tmp, i);
		cJSON* tmp3 = cJSON_GetObjectItem(tmp2, names[i]);
		utf8_to_gbk(tmp3->valuestring, gbk);
		fprintf(stdout, "key: %s, value: %s\n",names[i], gbk);
	}
 
	if (items != NULL) cJSON_Delete(items);
	if (json != NULL) free(json);
	if (printed_json != NULL) free(printed_json);
 
	return 0;
}
#else

int cli_json_test(int argc, char **argv)
{
    char *actual = NULL;
    cJSON *tree = NULL;

    if (argc < 2) {
        return CMD_ERR_PARAM;
    }

    /* read and parse test */
    tree = parse_file(argv[1]);
    if (tree == NULL ) {
        printf("Failed to read of parse test.\n");
        return 0;
    }

    /* print the parsed tree */
    actual = cJSON_Print(tree);
    if (actual != NULL ) {
        printf("file %s: \n", argv[1]);
        printf("%s\n", actual);
    }

    if (tree != NULL)
    {
        cJSON_Delete(tree);
    }
    
    if (actual != NULL)
    {
        free(actual);
    }

    return 0;
}
#endif

