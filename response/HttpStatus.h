#pragma once

enum HttpStatus {
  // 200x
  OK = 200, CREATED = 201, NO_CONTENT = 204,
  // 300x
  MOVED_PERMANENTLY = 301,
  // 400x
  BAD_REQUEST = 400, NOT_FOUND = 404, NOT_ALLOWED = 405,
  // 500x
  INTERNAL_SERVER_ERROR = 500
};
