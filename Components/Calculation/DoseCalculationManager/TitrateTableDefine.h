#pragma once

enum StepOption
{
    STEP_OPTION_ONE,
    STEP_OPTION_TWO,
    STEP_OPTION_THREE,
    STEP_OPTION_FOUR,
    STEP_OPTION_FIVE,
    STEP_OPTION_SIX,
    STEP_OPTION_SEVEN,
    STEP_OPTION_EIGHT,
    STEP_OPTION_NINE,
    STEP_OPTION_TEN,
    STEP_OPTION_NR,
};

enum DatumTerm
{
    DATUM_TERM_DOSE,
    DATUM_TERM_INFUSIONRATE,
    DATUM_TERM_DRIPRATE,
    DATUM_TERM_NR
};

enum DoseType
{
    DOSE_TYPE_MIN,
    DOSE_TYPE_H,
    DOSE_TYPE_KGMIN,
    DOSE_TYPE_KGH,
    DOSE_TYPE_NR
};

struct SetTableParam
{
    SetTableParam()
    {
        step = STEP_OPTION_ONE;
        datumTerm = DATUM_TERM_DOSE;
        doseType = DOSE_TYPE_H;
    }
    StepOption step;
    DatumTerm datumTerm;
    DoseType doseType;
};

class TitrateTableDefine
{
public:
    static const char *convert(DatumTerm index)
    {
        static const char *symbol[DATUM_TERM_NR] =
        {
            "Dosemin",
            "InfusionRate",
            "DripRate"
        };
        return symbol[index];
    }

    static const char *convert(DoseType index)
    {
        static const char *symbol[DOSE_TYPE_NR] =
        {
            "minDose",
            "hDose",
            "kgminDose",
            "kghDose"
        };
        return symbol[index];
    }

    static const int &convert(StepOption index)
    {
        static const int symbol[STEP_OPTION_NR] =
        {
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10
        };
        return symbol[index];
    }
};
