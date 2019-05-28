/*==============================================*/
/* Copyright (c)  2018-2019  NextMove Software  */
/*                                              */
/* This file is part of molhash.                */
/*                                              */
/* The contents are covered by the terms of the */
/* BSD license, which is included in the file   */
/* license.txt.                                 */
/*==============================================*/
#include <stdio.h>

#include <RDGeneral/versions.h>

int main()
{
  unsigned int parts[3] = {0, 0, 0};

  const char* version = RDKit::rdkitVersion;
  const char *p = version;
  unsigned int part = 0;
  while(*p) {
    if (*p >= '0' && *p <= '9') {
      parts[part] *= 10;
      parts[part] += *p - '0';
    } else if ( *p == '.') {
      part++;
      if (part == 3)
        break;
    } else {
      break; // e.g. the 'd' in "2019.03.01dev"
    }
    p++;
  }

  printf("%d", parts[0]*10000 + parts[1]*100 + parts[2]);
  return 0;
}

