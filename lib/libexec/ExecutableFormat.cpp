#include"Global.h"
#include "stl/slinkedlist.h"
#include"ExecutableFormat.h"
#include"ELF.h"

ExecutableFormat::ExecutableFormat(const char *p) : path(p)
{
}

ExecutableFormat::ExecutableFormat(void *m) : mem(m)
{
}

ExecutableFormat::~ExecutableFormat()
{
}

ExecutableFormat * ExecutableFormat::findForPath(const char *path)
{
/*
 *    ExecutableFormat *fmt = ZERO;
 *    List<FormatDetectorForPath> formats;
 *    struct stat st;
 *
 *    [> Insert known formats. <]
 *    formats.insertTail(ELF::detectPath);
 *
 *    [> Must be an existing, regular, executable file. <]
 *    if (stat(path, &st) != -1)
 *    {
 *        if (!S_ISREG(st.st_mode))
 *        {
 *            errno = EINVAL;
 *            return ZERO;
 *        }
 *    }
 *    else
 *        return ZERO;
 *
 *    [> Search for the corresponding executable format. <]
 *    for (ListIterator<FormatDetector> i(&formats); i.hasNext(); i++)
 *    {
 *    if ((fmt = (i.current())(path)))
 *    {
 *        return fmt;
 *    }
 *    }
 *    errno = ENOEXEC;
 *    return ZERO;
 */
	return nullptr;
}

ExecutableFormat * ExecutableFormat::findForMem(void* mem)
{
    ExecutableFormat *fmt = nullptr;
	lr::sstl::List<FormatDetectorForMem> formats;

    /* Insert known formats. */
    formats.PushBack(ELF::detectMem);

    /* Search for the corresponding executable format. */
	for(auto& i : formats)
    {
		if ((fmt = i(mem)))
		{
			return fmt;
		}
    }
    return nullptr;
}
