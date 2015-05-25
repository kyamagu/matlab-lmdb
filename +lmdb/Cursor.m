classdef Cursor < handle
%CURSOR LMDB cursor wrapper.
%
% Usage:
%
% transaction = database.begin('RDONLY', true);
% cursor = transaction.cursor();
% while cursor.next()
%   key = cursor.key;
%   value = cursor.value;
% end
% clear cursor;
% transaction.commit();
%
% See also lmdb

properties (Access = private)
  id_ % ID of the session.
  transaction_id_ % ID of the transaction session.
  database_id_ % ID of the database session.
end

properties (Dependent)
  key
  value
end

methods (Hidden)
  function this = Cursor(transaction_id, database_id, varargin)
  %CURSOR Create a new cursor.
    assert(isscalar(this));
    assert(isscalar(database_id));
    this.id_ = LMDB_('cursor_new', transaction_id, database_id, varargin{:});
    this.transaction_id_ = transaction_id;
    this.database_id_ = database_id;
  end
end

methods
  function delete(this)
  %DELETE Destructor.
    assert(isscalar(this));
    LMDB_('cursor_delete', this.id_);
  end

  function flag = next(this)
  %NEXT Proceed to next and return true if the next value exists.
    assert(isscalar(this));
    flag = LMDB_('cursor_next', this.id_);
  end

  function flag = previous(this)
  %PREVIOUS Proceed to previous and return true if the next value exists.
    assert(isscalar(this));
    flag = LMDB_('cursor_previous', this.id_);
  end

  function key_value = get.key(this)
  %GETKEY Return the current key.
    key_value = LMDB_('cursor_getkey', this.id_);
  end

  function value_value = get.value(this)
  %GETVALUE Return the current value.
    value_value = LMDB_('cursor_getvalue', this.id_);
  end
end

end
