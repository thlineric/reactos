/*
 * PROJECT:         Win32 subsystem
 * LICENSE:         See COPYING in the top level directory
 * FILE:            win32ss/gdi/dib/dib1bpp.c
 * PURPOSE:         Device Independant Bitmap functions, 1bpp
 * PROGRAMMERS:     Jason Filby
 *                  Doug Lyons
 */

#include <win32k.h>

#define NDEBUG
#include <debug.h>

#define DEC_OR_INC(var, decTrue, amount) \
    ((var) = (decTrue) ? ((var) - (amount)) : ((var) + (amount)))

VOID
DIB_1BPP_PutPixel(SURFOBJ *SurfObj, LONG x, LONG y, ULONG c)
{
  PBYTE addr = (PBYTE)SurfObj->pvScan0 + y * SurfObj->lDelta + (x >> 3);

  if (0 == (c & 0x01))
    *addr &= ~MASK1BPP(x);
  else
    *addr |= MASK1BPP(x);
}

ULONG
DIB_1BPP_GetPixel(SURFOBJ *SurfObj, LONG x, LONG y)
{
  PBYTE addr = (PBYTE)SurfObj->pvScan0 + y * SurfObj->lDelta + (x >> 3);

  return (*addr & MASK1BPP(x) ? 1 : 0);
}

VOID
DIB_1BPP_HLine(SURFOBJ *SurfObj, LONG x1, LONG x2, LONG y, ULONG c)
{
  while(x1 < x2)
  {
    DIB_1BPP_PutPixel(SurfObj, x1, y, c);
    x1++;
  }
}

VOID
DIB_1BPP_VLine(SURFOBJ *SurfObj, LONG x, LONG y1, LONG y2, ULONG c)
{
  while(y1 < y2)
  {
    DIB_1BPP_PutPixel(SurfObj, x, y1, c);
    y1++;
  }
}

static
void
DIB_1BPP_BitBltSrcCopy_From1BPP (
                                 SURFOBJ* DestSurf,
                                 SURFOBJ* SourceSurf,
                                 XLATEOBJ* pxlo,
                                 PRECTL DestRect,
                                 POINTL *SourcePoint,
                                 BOOLEAN bTopToBottom,
                                 BOOLEAN bLeftToRight )
{

  DPRINT("bLeftToRight is '%d' and bTopToBottom is '%d'.\n", bLeftToRight, bTopToBottom);

  // The 'window' in this sense is the x-position that corresponds
  // to the left-edge of the 8-pixel byte we are currently working with.
  // dwx is current x-window, dwx2 is the 'last' window we need to process.
  int dwx, dwx2;  // Destination window x-position
  int swx;        // Source window x-position

  // Left and right edges of source and dest rectangles
  int dl = DestRect->left; // dest left
  int dr = DestRect->right-1; // dest right (inclusive)
  int sl = SourcePoint->x; // source left
  int sr = sl + dr - dl; // source right (inclusive)

  // Which direction are we going?
  int xinc;
  int yinc;
  int ySrcDelta, yDstDelta;

  // The following 4 variables are used for the y-sweep
  int dy;  // dest y
  int dy1; // dest y start
  int dy2; // dest y end
  int sy1; // src  y start

  int shift;
  BYTE srcmask, dstmask, xormask;

  // 'd' and 's' are the dest & src buffer pointers that I use on my x-sweep
  // 'pd' and 'ps' are the dest & src buffer pointers used on the inner y-sweep
  PBYTE d, pd; // dest ptrs
  PBYTE s, ps; // src ptrs

  shift = (dl-sl)&7;

  xormask = 0xFF * (BYTE)XLATEOBJ_iXlate(pxlo, 0);

  if ( DestRect->top <= SourcePoint->y )
  {
    DPRINT("Moving up (scan top -> bottom).\n");
    // Moving up (scan top -> bottom)
    dy1 = DestRect->top;
    dy2 = DestRect->bottom - 1;
    if (bTopToBottom)
    {
      sy1 = SourcePoint->y + dy2 - dy1;
      ySrcDelta = -SourceSurf->lDelta;
    }
    else
    {
      sy1 = SourcePoint->y;
      ySrcDelta = SourceSurf->lDelta;
    }
    yinc = 1;
    yDstDelta = DestSurf->lDelta;
  }
  else
  {
    DPRINT("Moving down (scan bottom -> top).\n");
    // Moving down (scan bottom -> top)
    dy1 = DestRect->bottom - 1;
    dy2 = DestRect->top;
    if (bTopToBottom)
    {
      sy1 = SourcePoint->y;
      ySrcDelta = SourceSurf->lDelta;
    }
    else
    {
      sy1 = SourcePoint->y + dy1 - dy2;
      ySrcDelta = -SourceSurf->lDelta;
    }
    yinc = -1;
    yDstDelta = -DestSurf->lDelta;
  }
  if ( DestRect->left <= SourcePoint->x )
  {
    DPRINT("Moving left (scan left->right).\n");
    // Moving left (scan left->right)
    dwx = dl&~7;
    dwx2 = dr&~7;
    if (bLeftToRight)
    {
      swx = (sr - (dr & 7)) & ~7;
      xinc = -1;
    }
    else
    {
      swx = (sl-(dl&7))&~7;
      xinc = 1;
    }
  }
  else
  {
    DPRINT("Moving right (scan right->left).\n");
    // Moving right (scan right->left)
    dwx = dr & ~7;
    dwx2 = dl & ~7;
    if (bLeftToRight)
    {
      swx = (sl-(dl&7))&~7;
      xinc = 1;
    }
    else
    {
      swx = (sr - (dr & 7)) & ~7; // (sr - 7) & ~7; // We need the left edge of this block. Thus the -7
      xinc = -1;
    }
  }
  d = &(((PBYTE)DestSurf->pvScan0)[dy1*DestSurf->lDelta + (dwx>>3)]);
  s = &(((PBYTE)SourceSurf->pvScan0)[sy1*SourceSurf->lDelta + (swx>>3)]);
  for ( ;; )
  {
    dy = dy1;
    pd = d;
    ps = s;
    srcmask = 0xff;
    if ( dwx < dl )
    {
      int diff = dl-dwx;
      srcmask &= (1<<(8-diff))-1;
    }
    if ( dwx+7 > dr )
    {
      int diff = dr-dwx+1;
      srcmask &= ~((1<<(8-diff))-1);
    }
    dstmask = ~srcmask;

    // We unfortunately *must* have 5 different versions of the inner
    // loop to be certain we don't try to read from memory that is not
    // needed and may in fact be invalid.
    if ( !shift )
    {
      for ( ;; )
      {
        *pd = (BYTE)((*pd & dstmask) | ((ps[0]^xormask) & srcmask));

        // This *must* be here, because we could be going up *or* down...
        if ( dy == dy2 )
          break;
        dy += yinc;
        pd += yDstDelta;
        ps += ySrcDelta;
      }
    }
    else if ( !(0xFF00 & (srcmask<<shift) ) ) // Check if ps[0] not needed...
    {
      for ( ;; )
      {
        *pd = (BYTE)((*pd & dstmask)
          | ( ( (ps[1]^xormask) >> shift ) & srcmask ));

        // This *must* be here, because we could be going up *or* down...
        if ( dy == dy2 )
          break;
        dy += yinc;
        pd += yDstDelta;
        ps += ySrcDelta;
      }
    }
    else if ( !(0xFF & (srcmask<<shift) ) ) // Check if ps[1] not needed...
    {
      for ( ;; )
      {
        *pd = (*pd & dstmask)
          | ( ( (ps[0]^xormask) << ( 8 - shift ) ) & srcmask );

        // This *must* be here, because we could be going up *or* down...
        if ( dy == dy2 )
          break;
        dy += yinc;
        pd += yDstDelta;
        ps += ySrcDelta;
      }
    }
    else // Both ps[0] and ps[1] are needed
    {
      for ( ;; )
      {
        *pd = (*pd & dstmask)
          | ( ( ( ((ps[1]^xormask))|((ps[0]^xormask)<<8) ) >> shift ) & srcmask );

        // This *must* be here, because we could be going up *or* down...
        if ( dy == dy2 )
          break;
        dy += yinc;
        pd += yDstDelta;
        ps += ySrcDelta;
      }
    }

    // This *must* be here, because we could be going right *or* left...
    if ( dwx == dwx2 )
      break;
    d += xinc;
    s += xinc;
    dwx += xinc<<3;
    swx += xinc<<3;
  }
}

BOOLEAN
DIB_1BPP_BitBltSrcCopy(PBLTINFO BltInfo)
{
  ULONG Color;
  LONG i, j, sx, sy;
  BOOLEAN bTopToBottom, bLeftToRight;

  // This sets sy to the top line
  sy = BltInfo->SourcePoint.y;

  DPRINT("DIB_1BPP_BitBltSrcCopy: SrcSurf cx/cy (%d/%d), DestSuft cx/cy (%d/%d) dstRect: (%d,%d)-(%d,%d)\n",
         BltInfo->SourceSurface->sizlBitmap.cx, BltInfo->SourceSurface->sizlBitmap.cy,
         BltInfo->DestSurface->sizlBitmap.cx, BltInfo->DestSurface->sizlBitmap.cy,
         BltInfo->DestRect.left, BltInfo->DestRect.top, BltInfo->DestRect.right, BltInfo->DestRect.bottom);

  /* Get back left to right flip here */
  bLeftToRight = (BltInfo->DestRect.left > BltInfo->DestRect.right);

  /* Check for top to bottom flip needed. */
  bTopToBottom = BltInfo->DestRect.top > BltInfo->DestRect.bottom;

  // Make WellOrdered with top < bottom and left < right
  RECTL_vMakeWellOrdered(&BltInfo->DestRect);

  DPRINT("BPP is '%d' & BltInfo->SourcePoint.x is '%d' & BltInfo->SourcePoint.y is '%d'.\n",
         BltInfo->SourceSurface->iBitmapFormat, BltInfo->SourcePoint.x, BltInfo->SourcePoint.y);

  switch ( BltInfo->SourceSurface->iBitmapFormat )
  {
  case BMF_1BPP:
    DPRINT("1BPP Case Selected with DestRect Width of '%d'.\n",
           BltInfo->DestRect.right - BltInfo->DestRect.left);

    DIB_1BPP_BitBltSrcCopy_From1BPP ( BltInfo->DestSurface, BltInfo->SourceSurface,
      BltInfo->XlateSourceToDest, &BltInfo->DestRect, &BltInfo->SourcePoint,
      bTopToBottom, bLeftToRight );
    break;

  case BMF_4BPP:
    DPRINT("4BPP Case Selected with DestRect Width of '%d'.\n",
           BltInfo->DestRect.right - BltInfo->DestRect.left);

    if (bTopToBottom)
    {
      // This sets sy to the bottom line
      sy += (BltInfo->DestRect.bottom - BltInfo->DestRect.top - 1) * BltInfo->SourceSurface->lDelta;
    }

    for (j=BltInfo->DestRect.top; j<BltInfo->DestRect.bottom; j++)
    {
      sx = BltInfo->SourcePoint.x;

      if (bLeftToRight)
      {
       // This sets the sx to the rightmost pixel
       sx += (BltInfo->DestRect.right - BltInfo->DestRect.left - 1);
      }

      for (i=BltInfo->DestRect.left; i<BltInfo->DestRect.right; i++)
      {
        Color = XLATEOBJ_iXlate(BltInfo->XlateSourceToDest, DIB_4BPP_GetPixel(BltInfo->SourceSurface, sx, sy));
        DIB_1BPP_PutPixel(BltInfo->DestSurface, i, j, Color);

        DEC_OR_INC(sx, bLeftToRight, 1);
      }
      DEC_OR_INC(sy, bTopToBottom, 1);
    }
    break;

  case BMF_8BPP:
    DPRINT("8BPP-dstRect: (%d,%d)-(%d,%d) and Width of '%d'.\n", 
           BltInfo->DestRect.left, BltInfo->DestRect.top,
           BltInfo->DestRect.right, BltInfo->DestRect.bottom,
           BltInfo->DestRect.right - BltInfo->DestRect.left);
 
    if (bTopToBottom)
    {
      // This sets sy to the bottom line
      sy += (BltInfo->DestRect.bottom - BltInfo->DestRect.top - 1) * BltInfo->SourceSurface->lDelta;
    }

    for (j=BltInfo->DestRect.top; j<BltInfo->DestRect.bottom; j++)
    {
      sx = BltInfo->SourcePoint.x;

      if (bLeftToRight)
      {
        // This sets sx to the rightmost pixel
        sx += (BltInfo->DestRect.right - BltInfo->DestRect.left - 1);
      }

      for (i=BltInfo->DestRect.left; i<BltInfo->DestRect.right; i++)
      {
        Color = XLATEOBJ_iXlate(BltInfo->XlateSourceToDest, DIB_8BPP_GetPixel(BltInfo->SourceSurface, sx, sy));
        DIB_1BPP_PutPixel(BltInfo->DestSurface, i, j, Color);

        DEC_OR_INC(sx, bLeftToRight, 1);
      }
      DEC_OR_INC(sy, bTopToBottom, 1);
    }
    break;

  case BMF_16BPP:
    DPRINT("16BPP-dstRect: (%d,%d)-(%d,%d) and Width of '%d'.\n", 
           BltInfo->DestRect.left, BltInfo->DestRect.top,
           BltInfo->DestRect.right, BltInfo->DestRect.bottom,
           BltInfo->DestRect.right - BltInfo->DestRect.left);

    if (bTopToBottom)
    {
      // This sets sy to the bottom line
      sy += (BltInfo->DestRect.bottom - BltInfo->DestRect.top - 1) * BltInfo->SourceSurface->lDelta;;
    }

    for (j=BltInfo->DestRect.top; j<BltInfo->DestRect.bottom; j++)
    {
      sx = BltInfo->SourcePoint.x;

      if (bLeftToRight)
      {
        // This sets the sx to the rightmost pixel
        sx += (BltInfo->DestRect.right - BltInfo->DestRect.left - 1);
      }

      for (i=BltInfo->DestRect.left; i<BltInfo->DestRect.right; i++)
      {
        Color = XLATEOBJ_iXlate(BltInfo->XlateSourceToDest, DIB_16BPP_GetPixel(BltInfo->SourceSurface, sx, sy));
        DIB_1BPP_PutPixel(BltInfo->DestSurface, i, j, Color);
        DEC_OR_INC(sx, bLeftToRight, 1);
      }
      DEC_OR_INC(sy, bTopToBottom, 1);
    }
    break;

  case BMF_24BPP:

    DPRINT("24BPP-dstRect: (%d,%d)-(%d,%d) and Width of '%d'.\n", 
           BltInfo->DestRect.left, BltInfo->DestRect.top,
           BltInfo->DestRect.right, BltInfo->DestRect.bottom,
           BltInfo->DestRect.right - BltInfo->DestRect.left);

      if (bTopToBottom)
      {
        // This sets sy to the bottom line
        sy += (BltInfo->DestRect.bottom - BltInfo->DestRect.top - 1) * BltInfo->SourceSurface->lDelta;
      }

    for (j=BltInfo->DestRect.top; j<BltInfo->DestRect.bottom; j++)
    {
      sx = BltInfo->SourcePoint.x;

      if (bLeftToRight)
      {
        // This sets the sx to the rightmost pixel
        sx += (BltInfo->DestRect.right - BltInfo->DestRect.left - 1);
      }

      for (i=BltInfo->DestRect.left; i<BltInfo->DestRect.right; i++)
      {
        Color = XLATEOBJ_iXlate(BltInfo->XlateSourceToDest, DIB_24BPP_GetPixel(BltInfo->SourceSurface, sx, sy));
        DIB_1BPP_PutPixel(BltInfo->DestSurface, i, j, Color);
        DEC_OR_INC(sx, bLeftToRight, 1);
      }
      DEC_OR_INC(sy, bTopToBottom, 1);
    }
    break;

  case BMF_32BPP:

    DPRINT("32BPP-dstRect: (%d,%d)-(%d,%d) and Width of '%d'.\n", 
           BltInfo->DestRect.left, BltInfo->DestRect.top,
           BltInfo->DestRect.right, BltInfo->DestRect.bottom,
           BltInfo->DestRect.right - BltInfo->DestRect.left);

    if (bTopToBottom)
    {
      // This sets sy to the bottom line
      sy += BltInfo->DestRect.bottom - BltInfo->DestRect.top - 1;
    }

    for (j=BltInfo->DestRect.top; j<BltInfo->DestRect.bottom; j++)
    {
      sx = BltInfo->SourcePoint.x;

      if (bLeftToRight)
      {
        // This sets the sx to the rightmost pixel
        sx += (BltInfo->DestRect.right - BltInfo->DestRect.left - 1);
      }

      for (i=BltInfo->DestRect.left; i<BltInfo->DestRect.right; i++)
      {
        Color = XLATEOBJ_iXlate(BltInfo->XlateSourceToDest, DIB_32BPP_GetPixel(BltInfo->SourceSurface, sx, sy));
        DIB_1BPP_PutPixel(BltInfo->DestSurface, i, j, Color);
        DEC_OR_INC(sx, bLeftToRight, 1);
      }
      DEC_OR_INC(sy, bTopToBottom, 1);
    }
    break;

  default:
    DbgPrint("DIB_1BPP_BitBlt: Unhandled Source BPP: %u\n", BitsPerFormat(BltInfo->SourceSurface->iBitmapFormat));
    return FALSE;
  }

  return TRUE;
}

#ifndef _USE_DIBLIB_
BOOLEAN
DIB_1BPP_BitBlt(PBLTINFO BltInfo)
{
  LONG DestX, DestY;
  LONG SourceX, SourceY;
  LONG PatternY = 0;
  ULONG Dest, Source = 0, Pattern = 0;
  ULONG Index;
  BOOLEAN UsesSource;
  BOOLEAN UsesPattern;
  PULONG DestBits;
  LONG RoundedRight;

  UsesSource = ROP4_USES_SOURCE(BltInfo->Rop4);
  UsesPattern = ROP4_USES_PATTERN(BltInfo->Rop4);

  RoundedRight = BltInfo->DestRect.right -
    ((BltInfo->DestRect.right - BltInfo->DestRect.left) & 31);
  SourceY = BltInfo->SourcePoint.y;

  if (UsesPattern)
  {
    if (BltInfo->PatternSurface)
    {
      PatternY = (BltInfo->DestRect.top + BltInfo->BrushOrigin.y) %
        BltInfo->PatternSurface->sizlBitmap.cy;
    }
    else
    {
      /* FIXME: Shouldn't it be expanded? */
      if (BltInfo->Brush)
        Pattern = BltInfo->Brush->iSolidColor;
    }
  }

  for (DestY = BltInfo->DestRect.top; DestY < BltInfo->DestRect.bottom; DestY++)
  {
    DestX = BltInfo->DestRect.left;
    SourceX = BltInfo->SourcePoint.x;
    DestBits = (PULONG)(
      (PBYTE)BltInfo->DestSurface->pvScan0 +
      (BltInfo->DestRect.left >> 3) +
      DestY * BltInfo->DestSurface->lDelta);

    if (DestX & 31)
    {
#if 0
      /* FIXME: This case is completely untested!!! */

      Dest = *((PBYTE)DestBits);
      NoBits = 31 - (DestX & 31);

      if (UsesSource)
      {
        Source = 0;
        /* FIXME: This is incorrect! */
        for (Index = 31 - NoBits; Index >= 0; Index++)
          Source |= (DIB_GetSource(SourceSurf, SourceX + Index, SourceY, ColorTranslation) << (31 - Index));
      }

      if (BltInfo->PatternSurface)
      {
        Pattern = 0;
        for (k = 31 - NoBits; k >= 0; k++)
          Pattern |= (DIB_GetSourceIndex(PatternObj, (X + BrushOrigin.x + k) % PatternWidth, PatternY) << (31 - k));
      }

      Dest = DIB_DoRop(BltInfo->Rop4, Dest, Source, Pattern);
      Dest &= ~((1 << (31 - NoBits)) - 1);
      Dest |= *((PBYTE)DestBits) & ((1 << (31 - NoBits)) - 1);

      *DestBits = Dest;

      DestX += NoBits;
      SourceX += NoBits;
#endif
    }

    for (; DestX < RoundedRight; DestX += 32, DestBits++, SourceX += 32)
    {
      Dest = *DestBits;

      if (UsesSource)
      {
        Source = 0;
        for (Index = 0; Index < 8; Index++)
        {
          Source |= DIB_GetSource(BltInfo->SourceSurface, SourceX + Index, SourceY, BltInfo->XlateSourceToDest) << (7 - Index);
          Source |= DIB_GetSource(BltInfo->SourceSurface, SourceX + Index + 8, SourceY, BltInfo->XlateSourceToDest) << (8 + (7 - Index));
          Source |= DIB_GetSource(BltInfo->SourceSurface, SourceX + Index + 16, SourceY, BltInfo->XlateSourceToDest) << (16 + (7 - Index));
          Source |= DIB_GetSource(BltInfo->SourceSurface, SourceX + Index + 24, SourceY, BltInfo->XlateSourceToDest) << (24 + (7 - Index));
        }
      }

      if (BltInfo->PatternSurface)
      {
        Pattern = 0;
        for (Index = 0; Index < 8; Index++)
        {
          Pattern |= DIB_GetSourceIndex(BltInfo->PatternSurface, (DestX + BltInfo->BrushOrigin.x + Index) % BltInfo->PatternSurface->sizlBitmap.cx, PatternY) << (7 - Index);
          Pattern |= DIB_GetSourceIndex(BltInfo->PatternSurface, (DestX + BltInfo->BrushOrigin.x + Index + 8) % BltInfo->PatternSurface->sizlBitmap.cx, PatternY) << (8 + (7 - Index));
          Pattern |= DIB_GetSourceIndex(BltInfo->PatternSurface, (DestX + BltInfo->BrushOrigin.x + Index + 16) % BltInfo->PatternSurface->sizlBitmap.cx, PatternY) << (16 + (7 - Index));
          Pattern |= DIB_GetSourceIndex(BltInfo->PatternSurface, (DestX + BltInfo->BrushOrigin.x + Index + 24) % BltInfo->PatternSurface->sizlBitmap.cx, PatternY) << (24 + (7 - Index));
        }
      }

      *DestBits = DIB_DoRop(BltInfo->Rop4, Dest, Source, Pattern);
    }

    if (DestX < BltInfo->DestRect.right)
    {
      for (; DestX < BltInfo->DestRect.right; DestX++, SourceX++)
      {
        Dest = DIB_1BPP_GetPixel(BltInfo->DestSurface, DestX, DestY);

        if (UsesSource)
        {
          Source = DIB_GetSource(BltInfo->SourceSurface, SourceX, SourceY, BltInfo->XlateSourceToDest);
        }

        if (BltInfo->PatternSurface)
        {
          Pattern = DIB_GetSourceIndex(BltInfo->PatternSurface, (DestX + BltInfo->BrushOrigin.x) % BltInfo->PatternSurface->sizlBitmap.cx, PatternY);
        }

        DIB_1BPP_PutPixel(BltInfo->DestSurface, DestX, DestY, DIB_DoRop(BltInfo->Rop4, Dest, Source, Pattern) & 0xF);
      }
    }

    SourceY++;
    if (BltInfo->PatternSurface)
    {
      PatternY++;
      PatternY %= BltInfo->PatternSurface->sizlBitmap.cy;
    }
  }

  return TRUE;
}

/* BitBlt Optimize */
BOOLEAN
DIB_1BPP_ColorFill(SURFOBJ* DestSurface, RECTL* DestRect, ULONG color)
{
  LONG DestY;

  /* Make WellOrdered with top < bottom and left < right */
  RECTL_vMakeWellOrdered(DestRect);

  for (DestY = DestRect->top; DestY< DestRect->bottom; DestY++)
  {
    DIB_1BPP_HLine(DestSurface, DestRect->left, DestRect->right, DestY, color);
  }
  return TRUE;
}
#endif // !_USE_DIBLIB_

BOOLEAN
DIB_1BPP_TransparentBlt(SURFOBJ *DestSurf, SURFOBJ *SourceSurf,
                        RECTL*  DestRect,  RECTL *SourceRect,
                        XLATEOBJ *ColorTranslation, ULONG iTransColor)
{
  return FALSE;
}

/* EOF */
