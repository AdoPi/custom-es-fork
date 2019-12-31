//
// Created by bkg2k on 20/11/2019.
//
#pragma once

#include <string>

//! Help types
enum class HelpType
{
  AllDirections, //!< All 4 firections
  UpDown,        //!< Up and Down
  LeftRight,     //!< Left and Right
  A,             //!< Button A
  B,             //!< Button B
  X,             //!< Button X
  Y,             //!< Button Y
  L,             //!< Left Trigger
  R,             //!< Right Trigger
  Start,         //!< Start button
  Select,        //!< Select button
  __Count,       //!< Total count of elements
};

class Help
{
  private:;
    //! All type's texts
    std::string mTexts[(int)HelpType::__Count + 1];
    //! Active help types (bitmask of (1 << HelpTypes))
    int mMask;

  public:
    /*!
     * @brief Default constructor
     */
    Help() : mMask(0)
    {
    }

    /*!
     * @brief Clear all help types previously set
     * @return This Help
     */
    Help& Clear()
    {
      mMask = 0;
      return *this;
    }

    /*!
     * @brief Assign a text to a help type
     * @param type Type to assign text to
     * @param text Text to assign
     * @return This Help
     */
    Help& Set(HelpType type, const std::string& text)
    {
      mTexts[(int)type] = text;
      mMask |= 1 << (int)type;
      return *this;
    }

    /*!
     * @brief Check if the no help type is set
     * @return True if no type has been set
     */
    bool Empty() const { return mMask == 0; }

    /*!
     * @brief Check if a type has been set or not
     * @param type Help type to check
     * @return True if the help type has been set
     */
    bool IsSet(HelpType type) const
    {
      return (mMask & (1 << (int)type)) != 0;
    }

    const std::string& Text(HelpType type) const
    {
      if ((unsigned int)type >= (unsigned int)HelpType::__Count) return mTexts[(int)HelpType::__Count];
      return mTexts[(int)type];
    }

    /*!
     * @brief Return the total number of help types
     * @return Help type count
     */
    static int TypeCount()
    {
      return (int)HelpType::__Count;
    }

    /*!
     * @brief Get the Help type corresponding to index from 0 to TypeCount()-1
     * @param i Index
     * @return Help type. If the index is invalid, an empty type is returned
     */
    static HelpType TypeFromIndex(int i)
    {
      if ((unsigned int)i >= (unsigned int)HelpType::__Count) return HelpType::__Count;
      return (HelpType)i;
    }
};
