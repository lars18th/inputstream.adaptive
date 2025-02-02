/*
 *  Copyright (C) 2022 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "HEVCCodecHandler.h"

#include "../utils/log.h"
#include "../utils/Utils.h"

using namespace UTILS;

HEVCCodecHandler::HEVCCodecHandler(AP4_SampleDescription* sd) : CodecHandler(sd)
{
  if (AP4_HevcSampleDescription* hevcSampleDescription =
          AP4_DYNAMIC_CAST(AP4_HevcSampleDescription, m_sampleDescription))
  {
    m_extraData.SetData(hevcSampleDescription->GetRawBytes().GetData(),
                        hevcSampleDescription->GetRawBytes().GetDataSize());
    m_naluLengthSize = hevcSampleDescription->GetNaluLengthSize();
  }
}

bool HEVCCodecHandler::ExtraDataToAnnexB()
{
  if (AP4_HevcSampleDescription* hevcSampleDescription =
          AP4_DYNAMIC_CAST(AP4_HevcSampleDescription, m_sampleDescription))
  {
    const AP4_Array<AP4_HvccAtom::Sequence>& sequences = hevcSampleDescription->GetSequences();

    if (sequences.ItemCount() == 0)
    {
      LOG::LogF(LOGWARNING, "No available sequences for HEVC codec extra data");
      return false;
    }

    //calculate the size for annexb
    AP4_Size size{0};
    for (unsigned int i{0}; i < sequences.ItemCount(); ++i)
    {
      for (unsigned int j{0}; j < sequences[i].m_Nalus.ItemCount(); ++j)
      {
        size += sequences[i].m_Nalus[j].GetDataSize() + 4;
      }
    }

    m_extraData.SetDataSize(size);
    uint8_t* cursor(m_extraData.UseData());

    for (unsigned int i{0}; i < sequences.ItemCount(); ++i)
    {
      for (unsigned int j{0}; j < sequences[i].m_Nalus.ItemCount(); ++j)
      {
        cursor[0] = 0;
        cursor[1] = 0;
        cursor[2] = 0;
        cursor[3] = 1;
        memcpy(cursor + 4, sequences[i].m_Nalus[j].GetData(),
               sequences[i].m_Nalus[j].GetDataSize());
        cursor += sequences[i].m_Nalus[j].GetDataSize() + 4;
      }
    }
    LOG::LogF(LOGDEBUG, "Converted %lu bytes HEVC codec extradata", m_extraData.GetDataSize());
    return true;
  }
  LOG::LogF(LOGWARNING, "No HevcSampleDescription - annexb extradata not available");
  return false;
}

bool HEVCCodecHandler::GetInformation(kodi::addon::InputstreamInfo& info)
{
  bool isChanged = UpdateInfoCodecName(info, CODEC::FOURCC_HEVC);

  uint32_t fourcc{0};
  switch (m_sampleDescription->GetFormat())
  {
    case AP4_SAMPLE_FORMAT_HEV1:
      fourcc = CODEC::MakeFourCC(CODEC::FOURCC_HEV1);
      break;
    case AP4_SAMPLE_FORMAT_HVC1:
      fourcc = CODEC::MakeFourCC(CODEC::FOURCC_HVC1);
      break;
    case AP4_SAMPLE_FORMAT_DVHE:
      fourcc = CODEC::MakeFourCC(CODEC::FOURCC_DVHE);
      break;
    case AP4_SAMPLE_FORMAT_DVH1:
      fourcc = CODEC::MakeFourCC(CODEC::FOURCC_DVH1);
      break;
    default:
      break;
  }
  if (fourcc > 0 && info.GetCodecFourCC() != fourcc)
  {
    info.SetCodecFourCC(fourcc);
    isChanged = true;
  }

  if (info.GetFpsRate() == 0)
  {
    if (AP4_HevcSampleDescription* hevcSampleDescription =
            AP4_DYNAMIC_CAST(AP4_HevcSampleDescription, m_sampleDescription))
    {
      if (hevcSampleDescription->GetAverageFrameRate() > 0)
      {
        info.SetFpsRate(hevcSampleDescription->GetAverageFrameRate());
        info.SetFpsScale(256);
        isChanged = true;
      }
      else if (hevcSampleDescription->GetConstantFrameRate() > 0)
      {
        info.SetFpsRate(hevcSampleDescription->GetConstantFrameRate());
        info.SetFpsScale(256);
        isChanged = true;
      }
    }
  }
  return isChanged;
}
