#include "toys.h"

// Humor toys.h
struct toy_context toys;
char libbuf[4096], toybuf[4096];
void show_help(FILE *out) {;}
void toy_exec(char *argv[]) {;}

// Parse config files into data structures.

struct symbol {
  struct symbol *next;
  int enabled, help_indent;
  char *name, *depends;
  struct double_list *help;
} *sym;

char *trim(char *s)
{
  while (isspace(*s)) s++;

  return s;
}

char *keyword(char *name, char *line)
{
  int len = strlen(name);

  line = trim(line);
  if (strncmp(name, line, len)) return 0;
  line += len;
  if (*line && !isspace(*line)) return 0;
  line = trim(line);

  return line;
}

char *dlist_zap(struct double_list **help)
{
  struct double_list *dd = dlist_pop(help);
  char *s = dd->data;

  free(dd);
  
  return s;
}

int zap_blank_lines(struct double_list **help)
{
  int got = 0;

  while (*help) {
    char *s;

    s = trim((*help)->data);

    if (*s) break;
    got++;
    free(dlist_zap(help));
  }

  return got;
}

// Collect "-a blah" description lines following a blank line (or start).
// Returns array of removed lines with *len entries (0 for none).

// Moves *help to new start of text (in case dash lines were at beginning).
// Sets *from to where dash lines removed from (in case they weren't).
// Discards blank lines before and after dashlines.

// If no prefix, *help NULL. If no postfix, *from == *help
// if no dashlines returned *from == *help.

char **grab_dashlines(struct double_list **help, struct double_list **from,
                      int *len)
{
  struct double_list *dd;
  char *s, **list;
  int count = 0;

  *len = 0;
  zap_blank_lines(help);
  *from = *help;

  // Find start of dash block. Must be at start or after blank line.
  for (;;) {
    s = trim((*from)->data);
    if (*s == '-' && s[1] != '-' && !count) break;

    if (!*s) count = 0;
    else count++;

    *from = (*from)->next;
    if (*from == *help) return 0;
  }

  // If there was whitespace before this, zap it. This can't take out *help
  // because zap_blank_lines skipped blank lines, and we had to have at least
  // one non-blank line (a dash line) to get this far.
  while (!*trim((*from)->prev->data)) {
    *from = (*from)->prev;
    free(dlist_zap(from));
  }

  // Count number of dashlines, copy out to array, zap trailing whitespace
  // If *help was at start of dashblock, move it with *from
  count = 0;
  dd = *from;
  if (*help == *from) *help = 0;
  for (;;) {
   if (*trim(dd->data) != '-') break;
   count++;
   if (*from == (dd = dd->next)) break;
  }

  list = xmalloc(sizeof(char *)*count);
  *len = count;
  while (count) list[--count] = dlist_zap(from);

  return list;
}

void parse(char *filename)
{
  FILE *fp = xfopen(filename, "r");
  struct symbol *new = 0;

  for (;;) {
    char *s, *line = NULL;
    size_t len;

    // Read line, trim whitespace at right edge.
    if (getline(&line, &len, fp) < 1) break;
    s = line+strlen(line);
    while (--s >= line) {
      if (!isspace(*s)) break;
      *s = 0;
    }

    // source or config keyword at left edge?
    if (*line && !isspace(*line)) {
      if ((s = keyword("config", line))) {
        new = xzalloc(sizeof(struct symbol));
        new->next = sym;
        new->name = s;
        sym = new;
      } else if ((s = keyword("source", line))) parse(s);

      continue;
    }
    if (!new) continue;

    if (sym && sym->help_indent) {
      dlist_add(&(new->help), line);
      if (sym->help_indent < 0) {
        sym->help_indent = 0;
        while (isspace(line[sym->help_indent])) sym->help_indent++;
      }
    }
    else if ((s = keyword("depends", line)) && (s = keyword("on", s)))
      new->depends = s;
    else if (keyword("help", line)) sym->help_indent = -1;
  }

  fclose(fp);
}

int charsort(void *a, void *b)
{
  char *aa = a, *bb = b;

  if (*aa < *bb) return -1;
  if (*aa > *bb) return 1;
  return 0;
}

int dashsort(char **a, char **b)
{
  char *aa = *a, *bb = *b;

  if (aa[1] < bb[1]) return -1;
  if (aa[1] > bb[1]) return 1;
  return 0;
}

int dashlinesort(char **a, char **b)
{
  return strcmp(*a, *b);
}

int main(int argc, char *argv[])
{
  FILE *fp;

  if (argc != 3) {
    fprintf(stderr, "usage: config2help Config.in .config\n");
    exit(1);
  }

  // Read Config.in
  parse(argv[1]);

  // read .config
  fp = xfopen(argv[2], "r");
  for (;;) {
    char *line = NULL;
    size_t len;

    if (getline(&line, &len, fp) < 1) break;
    if (!strncmp("CONFIG_", line, 7)) {
      struct symbol *try;
      char *s = line+7;

      for (try=sym; try; try=try->next) {
        len = strlen(try->name);
        if (!strncmp(try->name, s, len) && s[len]=='=' && s[len+1]=='y') {
          try->enabled++;
          break;
        } 
      }
    }
  }

  // Collate help according to usage, depends, and .config

  // Loop through each entry, finding duplicate enabled "usage:" names
  // This is in reverse order, so last entry gets collated with previous
  // entry until we run out of matching pairs.
  for (;;) {
    struct symbol *throw = 0, *catch;
    char *this, *that, *cusage, *tusage, *name;
    int len;

    // find a usage: name and collate all enabled entries with that name
    for (catch = sym; catch; catch = catch->next) {
      if (catch->enabled != 1) continue;
      if (catch->help && (that = keyword("usage:", catch->help->data))) {
        struct double_list *cfrom, *tfrom, *anchor;
        char *try, **cdashlines, **tdashlines;
        int clen, tlen;

        // Align usage: lines, finding a matching pair so we can suck help
        // text out of throw into catch, copying from this to that
        if (!throw) name = that;
        else if (strncmp(name, that, len) || !isspace(that[len])) continue;
        catch->enabled++;
        while (!isspace(*that) && *that) that++;
        if (!throw) len = that-name;
        that = trim(that);
        if (!throw) {
          throw = catch;
          this = that;

          continue;
        }

        // Grab option description lines to collate from catch and throw
        tusage = dlist_zap(&throw->help);
        tdashlines = grab_dashlines(&throw->help, &tfrom, &tlen);
        cusage = dlist_zap(&catch->help);
        cdashlines = grab_dashlines(&catch->help, &cfrom, &clen);
        anchor = catch->help;

        // If we've got both, collate and alphebetize
        if (cdashlines && tdashlines) {
          char **new = xmalloc(sizeof(char *)*(clen+tlen));

          memcpy(new, cdashlines, sizeof(char *)*clen);
          memcpy(new+clen, tdashlines, sizeof(char *)*tlen);
          free(cdashlines);
          free(tdashlines);
          qsort(new, clen+tlen, sizeof(char *), (void *)dashlinesort);
          cdashlines = new;

        // If just one, make sure it's in catch.
        } else if (tdashlines) cdashlines = tdashlines;

        // If throw had a prefix, insert it before dashlines, with a
        // blank line if catch had a prefix.
        if (tfrom && tfrom != throw->help) {
          if (throw->help || catch->help) dlist_add(&cfrom, strdup(""));
          else {
            dlist_add(&cfrom, 0);
            anchor = cfrom->prev;
          }
          while (throw->help && throw->help != tfrom)
            dlist_add(&cfrom, dlist_zap(&throw->help));
          if (cfrom && cfrom->prev->data && *trim(cfrom->prev->data))
            dlist_add(&cfrom, strdup(""));
        }
        if (!anchor) {
          dlist_add(&cfrom, 0);
          anchor = cfrom->prev;
        }

        // Splice sorted lines back in place
        if (cdashlines) {
          tlen += clen;

          for (clen = 0; clen < tlen; clen++) 
            dlist_add(&cfrom, cdashlines[clen]);
        }

        // If there were no dashlines, text would be considered prefix, so
        // the list is definitely no longer empty, so discard placeholder.
        if (!anchor->data) dlist_zap(&anchor);

        // zap whitespace at end of catch help text
        while (!*trim(anchor->prev->data)) {
          anchor = anchor->prev;
          free(dlist_zap(&anchor));
        }

        // Append trailing lines.
        while (tfrom) dlist_add(&anchor, dlist_zap(&tfrom));

        // Collate first [-abc] option block in usage: lines
        try = 0;
        if (*this == '[' && this[1] == '-' && this[2] != '-' &&
            *that == '[' && that[1] == '-' && that[2] != '-')
        {
          char *from = this+2, *to = that+2;
          int ff = strcspn(from, " ]"), tt = strcspn(to, " ]");

          if (from[ff] == ']' && to[tt] == ']') {
            try = xmprintf("[-%.*s%.*s] ", ff, from, tt, to);
            qsort(try+2, ff+tt, 1, (void *)charsort);
            this = trim(this+ff+3);
            that = trim(that+tt+3);
          }
        }

        // The list is definitely no longer empty, so discard placeholder.
        if (!anchor->data) dlist_zap(&anchor);

        // Add new collated line (and whitespace).
        dlist_add(&anchor, xmprintf("%*cusage: %.*s %s%s%s%s",
                  catch->help_indent, ' ', len, name, try ? try : "",
                  this, *this ? " " : "", that));
        free(try);
        dlist_add(&anchor, strdup(""));
        free(cusage);
        free(tusage);
        throw->enabled = 0;
        throw = catch;
        throw->help = anchor->prev->prev;

        throw = catch;
        this = throw->help->data + throw->help_indent + 8 + len;
      }
    }

    // Did we find one?

    if (!throw) break;
  }

  // Print out help #defines
  while (sym) {
    struct double_list *dd;

    if (sym->help) {
      int i;
      char *s = xstrdup(sym->name);

      for (i = 0; s[i]; i++) s[i] = tolower(s[i]);
      printf("#define help_%s \"", s);
      free(s);

      dd = sym->help;
      for (;;) {
        i = sym->help_indent;

        // Trim leading whitespace
        s = dd->data;
        while (isspace(*s) && i) {
          s++;
          i--;
        }
        for (i=0; s[i]; i++) {
          if (s[i] == '"' || s[i] == '\\') putchar('\\');
          putchar(s[i]);
        }
        putchar('\\');
        putchar('n');
        dd = dd->next;
        if (dd == sym->help) break;
      }
      printf("\"\n\n");
    }
    sym = sym->next;
  }

  return 0;
}
