/*
 *  Copyright (C) 2022 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "../ADTSReader.h"
#include "../AdaptiveByteStream.h"
#include "../TSReader.h"
#include "SampleReader.h"

class ATTR_DLL_LOCAL CADTSSampleReader : public ISampleReader, public ADTSReader
{
public:
  CADTSSampleReader(AP4_ByteStream* input, AP4_UI32 streamId);

  bool IsStarted() const override { return m_started; }
  bool EOS() const override { return m_eos; }
  uint64_t DTS() const override { return m_pts; }
  uint64_t PTS() const override { return m_pts; }
  AP4_Result Start(bool& bStarted) override;
  AP4_Result ReadSample() override;
  void Reset(bool bEOS) override;
  bool GetInformation(kodi::addon::InputstreamInfo& info) override
  {
    return ADTSReader::GetInformation(info);
  }
  bool TimeSeek(uint64_t pts, bool preceeding) override;
  void SetPTSOffset(uint64_t offset) override { m_ptsOffs = offset; }
  uint64_t GetStartPTS() const override { return m_startPts; }
  void SetStartPTS(uint64_t pts) override { m_startPts = pts; }
  int64_t GetPTSDiff() const override { return m_ptsDiff; }
  uint32_t GetTimeScale() const override { return 90000; }
  AP4_UI32 GetStreamId() const override { return m_streamId; }
  AP4_Size GetSampleDataSize() const override { return GetPacketSize(); }
  const AP4_Byte* GetSampleData() const override { return GetPacketData(); }
  uint64_t GetDuration() const override { return (ADTSReader::GetDuration() * 100) / 9; }
  bool IsEncrypted() const override { return false; }

private:
  bool m_eos{false};
  bool m_started{false};
  AP4_UI32 m_streamId;
  uint64_t m_pts{0};
  int64_t m_ptsDiff{0};
  uint64_t m_ptsOffs{~0ULL};
  uint64_t m_startPts{STREAM_NOPTS_VALUE};
  CAdaptiveByteStream* m_adByteStream;
};
