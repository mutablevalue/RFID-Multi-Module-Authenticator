#include "auth.h"

#include <string.h>

#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *AuthNamespace = "auth";
static const char *AuthUidKey = "uid";

static const int MaxUidLen = 10;

static uint8_t enrolled_uid[10];
static int enrolled_uid_len = 0;
static bool has_enrolled_uid = false;

void auth_init(void) {
  esp_err_t err = nvs_flash_init();

  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ESP_ERROR_CHECK(nvs_flash_init());
  }

  nvs_handle_t handle;
  err = nvs_open(AuthNamespace, NVS_READONLY, &handle);

  if (err != ESP_OK) {
    has_enrolled_uid = false;
    return;
  }

  size_t uid_size = sizeof(enrolled_uid);
  err = nvs_get_blob(handle, AuthUidKey, enrolled_uid, &uid_size);

  if (err == ESP_OK && uid_size > 0 && uid_size <= MaxUidLen) {
    enrolled_uid_len = (int)uid_size;
    has_enrolled_uid = true;
  } else {
    enrolled_uid_len = 0;
    has_enrolled_uid = false;
  }

  nvs_close(handle);
}

bool auth_has_enrolled_uid(void) { return has_enrolled_uid; }

bool auth_enroll_uid(const uint8_t *uid, int uid_len) {
  if (uid == NULL || uid_len <= 0 || uid_len > MaxUidLen) {
    return false;
  }

  nvs_handle_t handle;
  esp_err_t err = nvs_open(AuthNamespace, NVS_READWRITE, &handle);

  if (err != ESP_OK) {
    return false;
  }

  err = nvs_set_blob(handle, AuthUidKey, uid, uid_len);

  if (err == ESP_OK) {
    err = nvs_commit(handle);
  }

  nvs_close(handle);

  if (err != ESP_OK) {
    return false;
  }

  memcpy(enrolled_uid, uid, uid_len);
  enrolled_uid_len = uid_len;
  has_enrolled_uid = true;

  return true;
}

bool auth_uid_allowed(const uint8_t *uid, int uid_len) {
  if (!has_enrolled_uid || uid == NULL || uid_len != enrolled_uid_len) {
    return false;
  }

  return memcmp(uid, enrolled_uid, uid_len) == 0;
}

bool auth_clear_enrolled_uid(void) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(AuthNamespace, NVS_READWRITE, &handle);

  if (err != ESP_OK) {
    return false;
  }

  err = nvs_erase_key(handle, AuthUidKey);

  if (err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND) {
    err = nvs_commit(handle);
  }

  nvs_close(handle);

  if (err != ESP_OK) {
    return false;
  }

  memset(enrolled_uid, 0, sizeof(enrolled_uid));
  enrolled_uid_len = 0;
  has_enrolled_uid = false;

  return true;
}
