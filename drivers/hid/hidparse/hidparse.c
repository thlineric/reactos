/*
 * PROJECT:     ReactOS Universal Serial Bus Bulk Enhanced Host Controller Interface
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        drivers/usb/hidparse/hidparse.c
 * PURPOSE:     HID Parser
 * PROGRAMMERS:
 *              Michael Martin (michael.martin@reactos.org)
 *              Johannes Anderwald (johannes.anderwald@reactos.org)
 */

#include "hidparse.h"

PVOID
NTAPI
AllocFunction(
    IN ULONG ItemSize)
{
    PVOID Item = ExAllocatePool(NonPagedPool, ItemSize);
    if (Item)
    {
        //
        // zero item
        //
        RtlZeroMemory(Item, ItemSize);
    }

    //
    // done
    //
    return Item;
}

VOID
NTAPI
FreeFunction(
    IN PVOID Item)
{
    //
    // free item
    //
    ExFreePool(Item);
}

VOID
NTAPI
ZeroFunction(
    IN PVOID Item,
    IN ULONG ItemSize)
{
    //
    // zero item
    //
    RtlZeroMemory(Item, ItemSize);
}

VOID
NTAPI
CopyFunction(
    IN PVOID Target,
    IN PVOID Source,
    IN ULONG Length)
{
    //
    // copy item
    //
    RtlCopyMemory(Target, Source, Length);
}

VOID
NTAPI
DebugFunction(
    IN LPCSTR FormatStr, ...)
{

    va_list args;
    unsigned int i;
     char printbuffer[1024];

     va_start(args, FormatStr);
     i = vsprintf(printbuffer, FormatStr, args);
     va_end(args);

     DbgPrint(printbuffer);
}

VOID
NTAPI
HidP_FreeCollectionDescription (
    IN PHIDP_DEVICE_DESC   DeviceDescription)
{
    HID_PARSER Parser;

    //
    // init parser
    //
    HidParser_InitParser(AllocFunction, FreeFunction, ZeroFunction, CopyFunction, DebugFunction, NULL, &Parser);

    //
    // free collection
    //
    HidParser_FreeCollectionDescription(&Parser, DeviceDescription);
}


HIDAPI
NTSTATUS
NTAPI
HidP_GetCaps(
    IN PHIDP_PREPARSED_DATA  PreparsedData,
    OUT PHIDP_CAPS  Capabilities)
{
    HID_PARSER Parser;

    //
    // init parser
    //
    HidParser_InitParser(AllocFunction, FreeFunction, ZeroFunction, CopyFunction, DebugFunction, PreparsedData, &Parser);

    //
    // get caps
    //
    return HidParser_GetCaps(&Parser, Capabilities);
}

NTSTATUS
NTAPI
HidP_GetCollectionDescription(
    IN PHIDP_REPORT_DESCRIPTOR ReportDesc,
    IN ULONG DescLength,
    IN POOL_TYPE PoolType,
    OUT PHIDP_DEVICE_DESC DeviceDescription)
{
    PHID_PARSER Parser;
    HIDPARSER_STATUS Status;

    //
    // first allocate the parser
    //
    Status = HidParser_AllocateParser(AllocFunction, FreeFunction, ZeroFunction, CopyFunction, DebugFunction, &Parser);
    if (Status != HIDPARSER_STATUS_SUCCESS)
    {
        //
        // not enough memory
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // get description;
    //
    Status = HidParser_GetCollectionDescription(Parser, ReportDesc, DescLength, PoolType, DeviceDescription);

    //
    // FIXME parser memory leak
    //
    return Status;
}

HIDAPI
ULONG
NTAPI
HidP_MaxUsageListLength(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage  OPTIONAL,
  IN PHIDP_PREPARSED_DATA  PreparsedData)
{
    HID_PARSER Parser;

    //
    // sanity check
    //
    ASSERT(ReportType == HidP_Input || ReportType == HidP_Output || ReportType == HidP_Feature);

    //
    // init parser
    //
    HidParser_InitParser(AllocFunction, FreeFunction, ZeroFunction, CopyFunction, DebugFunction, PreparsedData, &Parser);


    //
    // get usage length
    //
    return HidParser_MaxUsageListLength(&Parser, ReportType, UsagePage);
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetSpecificValueCaps(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection,
  IN USAGE  Usage,
  OUT PHIDP_VALUE_CAPS  ValueCaps,
  IN OUT PULONG  ValueCapsLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData)
{
    HID_PARSER Parser;

    //
    // sanity check
    //
    ASSERT(ReportType == HidP_Input || ReportType == HidP_Output || ReportType == HidP_Feature);

    //
    // init parser
    //
    HidParser_InitParser(AllocFunction, FreeFunction, ZeroFunction, CopyFunction, DebugFunction, PreparsedData, &Parser);

    //
    // get value caps
    //
    return HidParser_GetSpecificValueCaps(&Parser, ReportType, UsagePage, LinkCollection, Usage, ValueCaps, ValueCapsLength);
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetUsages(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection  OPTIONAL,
  OUT USAGE  *UsageList,
  IN OUT ULONG  *UsageLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN PCHAR  Report,
  IN ULONG  ReportLength)
{
    HID_PARSER Parser;

    //
    // sanity check
    //
    ASSERT(ReportType == HidP_Input || ReportType == HidP_Output || ReportType == HidP_Feature);

    //
    // init parser
    //
    HidParser_InitParser(AllocFunction, FreeFunction, ZeroFunction, CopyFunction, DebugFunction, PreparsedData, &Parser);

    //
    // get usages
    //
    return HidParser_GetUsages(&Parser, ReportType, UsagePage, LinkCollection, UsageList, UsageLength, Report, ReportLength);
}


#undef HidP_GetButtonCaps

HIDAPI
NTSTATUS
NTAPI
HidP_UsageListDifference(
  IN PUSAGE  PreviousUsageList,
  IN PUSAGE  CurrentUsageList,
  OUT PUSAGE  BreakUsageList,
  OUT PUSAGE  MakeUsageList,
  IN ULONG  UsageListLength)
{
    return HidParser_UsageListDifference(PreviousUsageList, CurrentUsageList, BreakUsageList, MakeUsageList, UsageListLength);
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetUsagesEx(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USHORT  LinkCollection,
  OUT PUSAGE_AND_PAGE  ButtonList,
  IN OUT ULONG  *UsageLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN PCHAR  Report,
  IN ULONG  ReportLength)
{
    return HidP_GetUsages(ReportType, HID_USAGE_PAGE_UNDEFINED, LinkCollection, (PUSAGE)ButtonList, UsageLength, PreparsedData, Report, ReportLength);
}

HIDAPI
NTSTATUS
NTAPI
HidP_UsageAndPageListDifference(
   IN PUSAGE_AND_PAGE  PreviousUsageList,
   IN PUSAGE_AND_PAGE  CurrentUsageList,
   OUT PUSAGE_AND_PAGE  BreakUsageList,
   OUT PUSAGE_AND_PAGE  MakeUsageList,
   IN ULONG  UsageListLength)
{
    return HidParser_UsageAndPageListDifference(PreviousUsageList, CurrentUsageList, BreakUsageList, MakeUsageList, UsageListLength);
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetScaledUsageValue(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection  OPTIONAL,
  IN USAGE  Usage,
  OUT PLONG  UsageValue,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN PCHAR  Report,
  IN ULONG  ReportLength)
{
    HID_PARSER Parser;

    //
    // sanity check
    //
    ASSERT(ReportType == HidP_Input || ReportType == HidP_Output || ReportType == HidP_Feature);

    //
    // init parser
    //
    HidParser_InitParser(AllocFunction, FreeFunction, ZeroFunction, CopyFunction, DebugFunction, PreparsedData, &Parser);

    //
    // get scaled usage value
    //
    return HidParser_GetScaledUsageValue(&Parser, ReportType, UsagePage, LinkCollection, Usage, UsageValue, Report, ReportLength);
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetButtonCaps(
    HIDP_REPORT_TYPE ReportType,
    PHIDP_BUTTON_CAPS ButtonCaps,
    PUSHORT ButtonCapsLength,
    PHIDP_PREPARSED_DATA PreparsedData)
{
    return HidP_GetSpecificButtonCaps(ReportType, HID_USAGE_PAGE_UNDEFINED, 0, 0, ButtonCaps, (PULONG)ButtonCapsLength, PreparsedData);
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetSpecificButtonCaps(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection,
  IN USAGE  Usage,
  OUT PHIDP_BUTTON_CAPS  ButtonCaps,
  IN OUT PULONG  ButtonCapsLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetData(
  IN HIDP_REPORT_TYPE  ReportType,
  OUT PHIDP_DATA  DataList,
  IN OUT PULONG  DataLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetExtendedAttributes(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  OUT PHIDP_EXTENDED_ATTRIBUTES  Attributes,
  IN OUT PULONG  LengthAttributes)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetLinkCollectionNodes(
    OUT PHIDP_LINK_COLLECTION_NODE  LinkCollectionNodes,
    IN OUT PULONG  LinkCollectionNodesLength,
    IN PHIDP_PREPARSED_DATA  PreparsedData)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetUsageValue(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection,
  IN USAGE  Usage,
  OUT PULONG  UsageValue,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NTAPI
HidP_SysPowerEvent (
    IN PCHAR HidPacket,
    IN USHORT HidPacketLength,
    IN PHIDP_PREPARSED_DATA Ppd,
    OUT PULONG OutputBuffer)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NTAPI
HidP_SysPowerCaps (
    IN PHIDP_PREPARSED_DATA Ppd,
    OUT PULONG OutputBuffer)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_GetUsageValueArray(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection  OPTIONAL,
  IN USAGE  Usage,
  OUT PCHAR  UsageValue,
  IN USHORT  UsageValueByteLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}


HIDAPI
NTSTATUS
NTAPI
HidP_UnsetUsages(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection,
  IN PUSAGE  UsageList,
  IN OUT PULONG  UsageLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN OUT PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_TranslateUsagesToI8042ScanCodes(
  IN PUSAGE  ChangedUsageList,
  IN ULONG  UsageListLength,
  IN HIDP_KEYBOARD_DIRECTION  KeyAction,
  IN OUT PHIDP_KEYBOARD_MODIFIER_STATE  ModifierState,
  IN PHIDP_INSERT_SCANCODES  InsertCodesProcedure,
  IN PVOID  InsertCodesContext)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_TranslateUsageAndPagesToI8042ScanCodes(
   IN PUSAGE_AND_PAGE  ChangedUsageList,
   IN ULONG  UsageListLength,
   IN HIDP_KEYBOARD_DIRECTION  KeyAction,
   IN OUT PHIDP_KEYBOARD_MODIFIER_STATE  ModifierState,
   IN PHIDP_INSERT_SCANCODES  InsertCodesProcedure,
   IN PVOID  InsertCodesContext)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_SetUsages(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection,
  IN PUSAGE  UsageList,
  IN OUT PULONG  UsageLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN OUT PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_SetUsageValueArray(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection  OPTIONAL,
  IN USAGE  Usage,
  IN PCHAR  UsageValue,
  IN USHORT  UsageValueByteLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  OUT PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_SetUsageValue(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection,
  IN USAGE  Usage,
  IN ULONG  UsageValue,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN OUT PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_SetScaledUsageValue(
  IN HIDP_REPORT_TYPE  ReportType,
  IN USAGE  UsagePage,
  IN USHORT  LinkCollection  OPTIONAL,
  IN USAGE  Usage,
  IN LONG  UsageValue,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN OUT PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_SetData(
  IN HIDP_REPORT_TYPE  ReportType,
  IN PHIDP_DATA  DataList,
  IN OUT PULONG  DataLength,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN OUT PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
ULONG
NTAPI
HidP_MaxDataListLength(
  IN HIDP_REPORT_TYPE  ReportType,
  IN PHIDP_PREPARSED_DATA  PreparsedData)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

HIDAPI
NTSTATUS
NTAPI
HidP_InitializeReportForID(
  IN HIDP_REPORT_TYPE  ReportType,
  IN UCHAR  ReportID,
  IN PHIDP_PREPARSED_DATA  PreparsedData,
  IN OUT PCHAR  Report,
  IN ULONG  ReportLength)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

#undef HidP_GetValueCaps

HIDAPI
NTSTATUS
NTAPI
HidP_GetValueCaps(
  HIDP_REPORT_TYPE ReportType,
  PHIDP_VALUE_CAPS ValueCaps,
  PULONG ValueCapsLength,
  PHIDP_PREPARSED_DATA PreparsedData)
{
    UNIMPLEMENTED
    ASSERT(FALSE);
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NTAPI
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegPath)
{

    DPRINT1("********* HID PARSE *********\n");
    return STATUS_SUCCESS;
}
