#include "ident.h"

// Define the global identifier table
std::vector<Ident> TID;

// Insert an identifier into TID if not present. Returns its index in TID.
int putIdent(const std::string& name) {
    // Check if identifier already exists
    for (size_t i = 0; i < TID.size(); ++i) {
        if (TID[i].name == name) {
            return i;
        }
    }
    // If not found, add a new entry (not yet declared)
    TID.emplace_back();
    TID.back().name = name;
    TID.back().declare = false;
    TID.back().assign = false;
    TID.back().type = LEX_NULL;
    return TID.size() - 1;
}
