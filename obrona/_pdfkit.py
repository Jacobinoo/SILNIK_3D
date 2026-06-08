# -*- coding: utf-8 -*-
"""
Wspolne narzedzia do budowania PDF-ow obronnych (reportlab + Platypus).
Rejestruje czcionki z polskimi znakami (Arial / Consolas) i daje proste
funkcje-helpery: H1/H2/H3, P, CODE, TREE, QA, BULLET.
"""
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import cm
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_LEFT, TA_CENTER
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Preformatted, Table, TableStyle,
    PageBreak, HRFlowable, KeepTogether
)
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
import os

# ---------- Czcionki z polskimi znakami ----------
FONTS = r"C:\Windows\Fonts"
pdfmetrics.registerFont(TTFont("PL",      os.path.join(FONTS, "arial.ttf")))
pdfmetrics.registerFont(TTFont("PL-B",    os.path.join(FONTS, "arialbd.ttf")))
pdfmetrics.registerFont(TTFont("PL-I",    os.path.join(FONTS, "ariali.ttf")))
pdfmetrics.registerFont(TTFont("Mono",    os.path.join(FONTS, "consola.ttf")))
pdfmetrics.registerFont(TTFont("Mono-B",  os.path.join(FONTS, "consolab.ttf")))

from reportlab.pdfbase.pdfmetrics import registerFontFamily
registerFontFamily("PL", normal="PL", bold="PL-B", italic="PL-I", boldItalic="PL-B")

# ---------- Kolory ----------
C_TITLE  = colors.HexColor("#1a3a5c")
C_H1     = colors.HexColor("#1a3a5c")
C_H2     = colors.HexColor("#2566a8")
C_H3     = colors.HexColor("#3a3a3a")
C_CODEBG = colors.HexColor("#f4f6f8")
C_CODEBD = colors.HexColor("#cfd8e0")
C_QBG    = colors.HexColor("#eef4fb")
C_QBD    = colors.HexColor("#aac4e0")
C_TREEBG = colors.HexColor("#f8f8f0")

# ---------- Style ----------
def make_styles():
    s = {}
    s["title"] = ParagraphStyle("title", fontName="PL-B", fontSize=24, leading=28,
                                textColor=C_TITLE, alignment=TA_LEFT, spaceAfter=4)
    s["subtitle"] = ParagraphStyle("subtitle", fontName="PL", fontSize=13, leading=17,
                                   textColor=C_H2, spaceAfter=2)
    s["small"] = ParagraphStyle("small", fontName="PL", fontSize=9, leading=12,
                                textColor=colors.grey)
    s["h1"] = ParagraphStyle("h1", fontName="PL-B", fontSize=16, leading=20,
                             textColor=C_H1, spaceBefore=16, spaceAfter=6)
    s["h2"] = ParagraphStyle("h2", fontName="PL-B", fontSize=13, leading=17,
                             textColor=C_H2, spaceBefore=12, spaceAfter=4)
    s["h3"] = ParagraphStyle("h3", fontName="PL-B", fontSize=11, leading=15,
                             textColor=C_H3, spaceBefore=8, spaceAfter=3)
    s["body"] = ParagraphStyle("body", fontName="PL", fontSize=10, leading=14.5,
                               alignment=TA_LEFT, spaceAfter=6)
    s["bullet"] = ParagraphStyle("bullet", fontName="PL", fontSize=10, leading=14,
                                 leftIndent=14, bulletIndent=4, spaceAfter=2)
    s["code"] = ParagraphStyle("code", fontName="Mono", fontSize=8.2, leading=11,
                               textColor=colors.HexColor("#10243a"))
    s["tree"] = ParagraphStyle("tree", fontName="Mono", fontSize=8.6, leading=12,
                               textColor=colors.HexColor("#243a10"))
    s["qq"] = ParagraphStyle("qq", fontName="PL-B", fontSize=10, leading=14,
                             textColor=C_TITLE, spaceAfter=2)
    s["aa"] = ParagraphStyle("aa", fontName="PL", fontSize=10, leading=14,
                             textColor=colors.black, spaceAfter=2)
    return s

S = make_styles()

def esc(t):
    return (t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"))

# ---------- Flowable helpers ----------
def H1(t):  return Paragraph(esc(t), S["h1"])
def H2(t):  return Paragraph(esc(t), S["h2"])
def H3(t):  return Paragraph(esc(t), S["h3"])
def P(t):   return Paragraph(t, S["body"])          # t moze zawierac <b>..</b>
def Praw(t):return Paragraph(esc(t), S["body"])
def SPACER(h=6): return Spacer(1, h)
def HR(): return HRFlowable(width="100%", thickness=0.6, color=C_QBD,
                            spaceBefore=6, spaceAfter=6)

def BULLET(items):
    out = []
    for it in items:
        out.append(Paragraph("&bull;&nbsp;&nbsp;" + it, S["bullet"]))
    return out

def CODE(code, caption=None):
    """Blok kodu w ramce z tlem."""
    flow = []
    if caption:
        flow.append(Paragraph("<i>" + esc(caption) + "</i>", S["small"]))
        flow.append(Spacer(1, 2))
    pre = Preformatted(code, S["code"])
    tbl = Table([[pre]], colWidths=[16.6 * cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, -1), C_CODEBG),
        ("BOX", (0, 0), (-1, -1), 0.6, C_CODEBD),
        ("LEFTPADDING", (0, 0), (-1, -1), 7),
        ("RIGHTPADDING", (0, 0), (-1, -1), 7),
        ("TOPPADDING", (0, 0), (-1, -1), 5),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 5),
    ]))
    flow.append(tbl)
    flow.append(Spacer(1, 6))
    return flow

def TREE(text, caption=None):
    flow = []
    if caption:
        flow.append(Paragraph("<i>" + esc(caption) + "</i>", S["small"]))
        flow.append(Spacer(1, 2))
    pre = Preformatted(text, S["tree"])
    tbl = Table([[pre]], colWidths=[16.6 * cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, -1), C_TREEBG),
        ("BOX", (0, 0), (-1, -1), 0.6, colors.HexColor("#cdd0b8")),
        ("LEFTPADDING", (0, 0), (-1, -1), 8),
        ("RIGHTPADDING", (0, 0), (-1, -1), 8),
        ("TOPPADDING", (0, 0), (-1, -1), 6),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 6),
    ]))
    flow.append(tbl)
    flow.append(Spacer(1, 8))
    return flow

def QA(q, a):
    """Jedno pytanie + odpowiedz w delikatnej ramce. Trzymane razem."""
    qpar = Paragraph("<b>P:</b> " + q, S["qq"])
    apar = Paragraph("<b>O:</b> " + a, S["aa"])
    tbl = Table([[qpar], [apar]], colWidths=[16.6 * cm])
    tbl.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, -1), C_QBG),
        ("BOX", (0, 0), (-1, -1), 0.5, C_QBD),
        ("LEFTPADDING", (0, 0), (-1, -1), 8),
        ("RIGHTPADDING", (0, 0), (-1, -1), 8),
        ("TOPPADDING", (0, 0), (-1, -1), 5),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 5),
    ]))
    return KeepTogether([tbl, Spacer(1, 6)])

# ---------- Naglowek / stopka strony ----------
def _header_footer(canvas, doc):
    canvas.saveState()
    canvas.setFont("PL", 8)
    canvas.setFillColor(colors.grey)
    # stopka: numer strony
    canvas.drawRightString(A4[0] - 2 * cm, 1.1 * cm, "str. %d" % doc.page)
    canvas.drawString(2 * cm, 1.1 * cm, doc._docTitle)
    # linia nad stopka
    canvas.setStrokeColor(C_QBD)
    canvas.setLineWidth(0.4)
    canvas.line(2 * cm, 1.4 * cm, A4[0] - 2 * cm, 1.4 * cm)
    canvas.restoreState()

def build(filename, doc_title, story):
    doc = SimpleDocTemplate(filename, pagesize=A4,
                            leftMargin=2 * cm, rightMargin=2 * cm,
                            topMargin=1.8 * cm, bottomMargin=1.8 * cm,
                            title=doc_title)
    doc._docTitle = doc_title
    doc.build(story, onFirstPage=_header_footer, onLaterPages=_header_footer)
    print("Zapisano:", filename)

def title_block(name, osoba, role, files_line):
    out = []
    out.append(Paragraph(esc(name), S["title"]))
    out.append(Paragraph(esc(osoba + " - " + role), S["subtitle"]))
    out.append(Paragraph(esc("Materialy do obrony projektu: Silnik 3D + Strzelnica (FreeGLUT / OpenGL)"),
                         S["small"]))
    out.append(Spacer(1, 4))
    out.append(Paragraph("<b>Twoje pliki:</b> " + esc(files_line), S["small"]))
    out.append(HR())
    return out
