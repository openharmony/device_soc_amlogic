/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef DISPLAY_BYTRACE_H
#define DISPLAY_BYTRACE_H
#include <string>
#include <hitrace_meter.h>

namespace OHOS {
namespace HDI {
namespace DISPLAY {
class DisplayBytrace {
public:
    DisplayBytrace(std::string value)
    {
        StartTrace(HITRACE_TAG_GRAPHIC_AGP, value.c_str());
    }

    ~DisplayBytrace()
    {
        FinishTrace(HITRACE_TAG_GRAPHIC_AGP);
    }
};
} // namespace DISPLAY
} // namespace HDI
} // namespace OHOS

#endif // DISPLAY_BYTRACE_H