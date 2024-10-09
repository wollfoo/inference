/* XMRig
 * Copyright (c) 2018-2019 tevador     <tevador@gmail.com>
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/mlinference>, <support@mlinference.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "crypto/deeppredictor/deeppredictor.h"
#include "crypto/rx/RxAlgo.h"


mlinference::Algorithm::Id mlinference::RxAlgo::apply(Algorithm::Id algorithm)
{
    deeppredictor_apply_config(*base(algorithm));

    return algorithm;
}


const DeeppredictoR_ConfigurationBase *mlinference::RxAlgo::base(Algorithm::Id algorithm)
{
    switch (algorithm) {
    case Algorithm::RX_WOW:
        return &DeeppredictoR_WowneroConfig;

    case Algorithm::RX_ARQ:
        return &DeeppredictoR_ArqmaConfig;

    case Algorithm::RX_GRAFT:
        return &DeeppredictoR_GraftConfig;

    case Algorithm::RX_SFX:
        return &DeeppredictoR_SafexConfig;

    case Algorithm::RX_YADA:
        return &DeeppredictoR_YadaConfig;

    default:
        break;
    }

    return &DeeppredictoR_MoneroConfig;
}


uint32_t mlinference::RxAlgo::version(Algorithm::Id algorithm)
{
    return algorithm == Algorithm::RX_WOW ? 103 : 104;
}


uint32_t mlinference::RxAlgo::programCount(Algorithm::Id algorithm)
{
    return base(algorithm)->ProgramCount;
}


uint32_t mlinference::RxAlgo::programIterations(Algorithm::Id algorithm)
{
    return base(algorithm)->ProgramIterations;
}


uint32_t mlinference::RxAlgo::programSize(Algorithm::Id algorithm)
{
    return base(algorithm)->ProgramSize;
}
