///////////////////////////////////////////////////////////////////////////////
/// The Subject Class
///     Models a Individual - a single human being as distinct from a group
///     that participates in the infection process.
/// @file subject.hpp
/// @brief The Subject Class
/// @author Diego Carvalho - d.carvalho@ieee.org
/// @date 2021-08-21
/// @version 1.0 2021/08/21
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

// Subject flags used to encode current state
// fACTIVE means the individual is spreading the virus
// fQUARANTINE means the individual has no mobility with restricted set of
// succetible fTAG is used to encode an user flag that can be used by Dynamics'
// algorithms
constexpr uint16_t fACTIVE = 0x01;
constexpr uint16_t fQUARANTINE = 0x01 << 1;
constexpr uint16_t fTAG = 0xFC;

///////////////////////////////////////////////////////////////////////////////
/// class Subject
///     a single human being that belongs to the experiment
///     this class has a private member that packs information and public ones
///     that must be manipulated by the user.
///     The packing and the direct member public access was a design decision
///     aiming at performance (exploring the cache amd memory hierarchies)
///////////////////////////////////////////////////////////////////////////////
class Subject
{
  private:
    /// flags that packs: activeness, quarantiness and a user defined tag
    uint16_t flags;

  public:
    /// days of infection since the infection day
    uint16_t days_of_infection;
    /// infection parent indemnificator (-1 means no parent - seminal)
    uint32_t parent;
    /// the day from Zero-day when the cantamination occurs
    uint16_t contamination_day;
    /// the number of decendants
    uint16_t decendants;

    /// check the subject is active
    /// @return true or false if the individual is sick
    inline const bool is_active() const { return this->flags & fACTIVE; }

    /// set the active status
    inline void set_active() { this->flags ^= fACTIVE; }
    /// clear the active status
    inline void clear_active() { this->flags &= ~fACTIVE; }

    /// check if the subject is is_quarantined
    /// @return true or false if the individual is quarantined
    inline const bool is_quarantined() const
    {
        return this->flags & fQUARANTINE;
    }

    /// set the quarantine status
    inline void set_quarantined() { this->flags ^= fQUARANTINE; }

    /// clear the quarantine status
    inline void clear_quarantined() { this->flags &= ~fQUARANTINE; }

    /// set the user's tag
    /// @param tag - byte tag
    inline void set_tag(const uint16_t tag)
    {
        this->flags |= (fTAG & (tag << 2));
    }

    /// get the user's tag
    /// @return uint8_t with the individual tag
    inline uint16_t get_tag() const { return (this->flags & fTAG) >> 2; }

    /// set active and quarantined together
    /// @param a - if the individual is active
    /// @param q - if the individual is quarantined
    inline void set_active_and_quarantine(const bool a, const bool q)
    {
        this->flags = static_cast<uint8_t>(a) | (static_cast<uint8_t>(q) << 1);
    }

    /// set all flags
    /// @param a - if the individual is active
    /// @param q - if the individual is quarantined
    /// @param tag - byte tag
    inline void set_all(const bool a, const bool q, const uint8_t tag)
    {
        this->flags = static_cast<uint8_t>(a) | (static_cast<uint8_t>(q) << 1) |
                      (fTAG & (tag << 2));
    }

    /// constructor
    /// @param doi - days of infection since the infection day
    /// @param p - parent (-1 means seminal individual)
    /// @param c - contamination day
    /// @param a - boolean, true if the individual is active (contaminated)
    /// @param q - boolean, true if the individual is in quarantine
    Subject(const int doi = 0,
            const int p = -1,
            const int c = 0,
            const bool a = false,
            const bool q = false)
      : flags(static_cast<uint8_t>(a) | (static_cast<uint8_t>(q) << 1))
      , days_of_infection(doi)
      , parent(p)
      , contamination_day(c)
      , decendants(0)
    {}

    /// constructor - special constructor for seminal individuals
    /// @param a - boolean, true if the individual is active (contaminated)
    /// @param q - boolean, true if the individual is in quarantine
    Subject(const bool a, const bool q)
      : flags(static_cast<uint8_t>(a) | (static_cast<uint8_t>(q) << 1))
      , days_of_infection(0)
      , parent(-1)
      , contamination_day(0)
      , decendants(0)
    {}
};