===============================================================================
                                   odf2txt
===============================================================================
-------------------------------------------------------------------------------
      An ODF to text convertor optimized for comparing document revisions
-------------------------------------------------------------------------------

Introduction
============

odf2txt is a tool to convert ODF documents into a text-only representation. It
is capable of extracting just the text (with extremely minimal formatting) from
any document, but its main goal is to extract a representation that is highly
useful for comparing revisions of the same document, e.g. as a textconv filter
for git diff of ODF documents in a git repository. It should work well with any
repository type, but it is most useful with a diff tool implementing the
patience algorithm or variant thereof (e.g. the histogram algorithm of git
diff) that can further operate in word-wise comparison mode.

Additionally, odf2txt is capable of operating in a mode that also extracts
document formatting information. In particular, odf2txt tries to extract
*enough* information to be able to usefully identify changes that affect only
formatting, while discarding uninteresting markup that would result in a diff
containing more changes than can be practically perused.

The history section (below) provides a more complete description of the
problems inherent to producing good output for diff-based comparisons. See
the USAGE.rst file (or the man page, which can be generate from the same) for a
complete explanation of how to use odf2txt.

Current Status
==============

The current version is quite useable for simple text documents (.odt), although
much of the style handling is still missing (also handling of e.g. lists and
tables). This support is expected to improve quickly.

Please write me if you have any feature requests or would like to contribute,
or if you believe you have found a bug. For bug reports, it is *extremely*
helpful if you can provide a sample document which can demonstrate the problem
you are seeing. (Reports of unhandled elements are also welcomed; I have
neither the inclanation nor time to write something that fully conforms to the
ODF specification and am immesurably more interested in handling the most
common pieces, in order to produce a tool that can handle the majority of real
world content, with the ability to tell you what it isn't handling in order to
avoid unintended changes in unhandled content types. Again, sample documents
are immensely helpful, and may be required if I am unable to guess how to
reasonably convert something from just the raw XML snippets.)

A Brief History
===============

In 2012, odt2txt - a C program written by Dennis Stosberg - was the recommended
means of converting ODT documents to text for consumption by diff. odt2txt is
small, simple, and gets the job done in many cases using nothing more than
libzip and regular expressions (and some encoding support).

However, odt2txt has drawbacks:

- It lacks the ability to show formatting of any kind.
- It lacks diff-friendly paragraph splitting.
- It tries to be clever with headers and sometimes fails.

It was the third point in particular that spawned odf2txt. odt2txt has a built
in feature (that cannot be disabled) of emphasizing paragraphs by underlining
them. However, because it uses regular expressions rather than an XML parser,
it gets confused by tags within header elements, which can cause it to
underline entire non-header paragraphs. Aesthetically, this isn't great, but
changes in text flow (which can cause soft page break elements to come and go
from within header elements) and text formatting can cause this 'glitch' to
affect some revisions, but not others... with disasterous effects on diff
output: a change many pages away can cause a switch in a botched header
underline. If the following paragraph has also changed, diff - even word-wise
histogram diff - can fail to localize the relevant change, resulting in the
entire paragraph being marked as a change (with an enormous run of '-'s to keep
it company), even if only a single word was added or removed.

Eventually, this sort of thing became unacceptable, and I broke down and built
my own odt2txt with an option to turn of header underlining.

This helped... but I knew I could do better.

Why Python?
-----------

Before odf2txt, I had a locally modified odt2txt. However, odt2txt is written
in C, in an unfamiliar coding style, with a home-grown Makefile and a series of
supporting functions that any decent library could eliminate (including a
bundled libzip). But it isn't terribly complicated, and my first thought was to
rewrite it in C++.

I got about as far as starting the skeleton of a CMake-based build system when
planning for the libraries I would use made me stop to think. I needed regular
expressions (which I could get by requiring C++11), but also libzip, and I was
already thinking about using a real XML parser. Qt also came to mind (portable,
could provide both XML and regular expressions portably and without requiring
C++11), but it was at that point I realized I was overengineering... Python
could provide me everything I needed, and a lot more besides.

As an interpreted language, a Python tool would be fully cross platform,
running anywhere that had Python, and (unlike Java) it wouldn't need to be
compiled. It wouldn't even need a build system; the "source" and "binary" could
be the same, and a single file to boot. (Realistically, there is documentation,
but in a pinch, one *could* get away with worrying about just the script
itself on one's own systems.) And Python already provided handling of zip
archives, regular expressions, and a SAX parser (not to mention a very nice
command line argument/option handling module).

Why not HTML?
-------------

The next item to address was style. Of course, one *could* use an existing
tool to convert to a textual markup format (e.g. RTF or HTML), but there were
problems here as well. The obvious choice, unoconv, was out of the question
because it needs a connection to a LibreOffice server, which effectively means
it can't be run if one is running LibreOffice normally (e.g. editing a
document). The odfpy suite, in the guise of odf2xhtml, offered another option,
but has the same problem of most ODF to HTML tools, and rather the opposite of
odt2txt: because it tries very hard to preserve the original format of the
document, it outputs too *much* markup. (In particular, it still outputs full
definitions for implicitly created styles, many of which do not actually
contribute meaningfully to the document's formatting.) Besides, most of the
HTML structure is not important for the purpose of comparing revisions, and
just makes diffs harder to read to no actual benefit.

And lastly, most such conversion programs retain an aspect of ODF that makes
them inherently less useful; lack of inline styles. In ODF, *all* formatting is
defined at the beginning of the document, and only referenced in the document
body. A diff telling you that a certain word no longer has the format "T37"
tells you that the formatting *changed*, but still isn't nearly as useful as it
should be.

Enter: odf2txt
--------------

From this was born the first version of odf2txt. The very first version had no
text extraction at all, but already implemented a SAX parser with the ability
to push and pop specialized contextual parsers, and did a halfway decent job of
converting text properties of defined styles to something almost like CSS. Like
odt2txt it also has the ability to simply dump the unparsed raw XML bytes from
the ODF container (but can dump styles.xml as well as content.xml). From there,
it quickly gained the ability to extract the text contend of the document,
apply some simple formatting (again, very much like odt2txt, albiet even less
aggressive about applying simple text formatting). HTML-like markup of headings
soon followed, as did the ability to convert inline styles to the same
pseudo-CSS and then inject them inline, allowing the output to provide the
definition of inline formatting at the point of use.

Then came tackling the last of the problems with odt2txt: giving the diff
algorithm the best possible chance to find unchanged hunks. With traditional
algorithms this is already a lost cause, but the patience algorithm (and the
histogram algorithm that extends it) can do much better... with a little hand
holding. The initial implementation of sentence breaking was non-optional and
dumb, but even the first naïve implementation greatly improved matters.

Patience works by identifying matching unique lines of content, regardless of
position within the document. With no wrapping, or static wrapping, this still
fares quite badly, but by breaking at sentence boundaries, unmodified sentences
can be matched, greatly improving the chances of producing a minimal diff that
shows only 'true' changes.

The Future
----------

Despite its flaws, some of the formatting features of odt2txt have their
usefulness, and I have given thought to eventually teaching odf2txt to output
several different styles of markup, such as formal reStructuredText and/or
MarkDown, and eventually perhaps simplified HTML. (In the latter case, the
objective would be to preserve the general document style in HTML that is
reasonably lean without additional intervention, rather than to achieve a
page-accurate rendering, as LibreOffice or odfpy are quite capable of producing
that sort of output already.)

License
=======

odf2txt is written by Matthew Woehlke, and is distributed under the terms of
the GNU General Public Licence. See COPYING.txt for the complete license. The
copyright statement in the script applies to all parts of the software,
including this document, unless otherwise stated.
