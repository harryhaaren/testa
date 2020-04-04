Feature:
  Scenario Outline: Test 1
    Given user has a valid card;
    And account balance is <balance_start>;
    When they insert the card;
    And withdraw <withdraw>;
    Then the ATM should return <dispense>;
    And the balance should be <balance_end>

    Examples:
      | balance_start | withdraw | dispense | balance_end |
      |     50        |    20    |    20    |     30      |
      |     70        |    20    |    20    |     50      |
      |     10        |     2    |     2    |      8      |
      |      1        |   300    |     0    |      1      |
